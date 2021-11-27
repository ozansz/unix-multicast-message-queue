#include "message_chain.h"

std::string MCCreds::Dump() {
    std::ostringstream stream;

    stream << "\nnodes_shmkey: " << this->nodes_shmkey;
    stream << "\nhead_index_shmkey: " << this->head_index_shmkey;
    stream << "\nactive_nodes_shmkey: " << this->active_nodes_shmkey;
    stream << "\n\nrwlock: \n" << this->rwcreds->Dump();

    stream << "\n";

    return stream.str();
}

MessageChain::MessageChain(const char *tag) {
    this->tag = tag;
    this->rwlock = new RWLock();
}

MessageChain::~MessageChain() {
    if (shmdt(nodes) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (shmdt(head_index) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (shmdt(active_nodes) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (rwlock->Clear() > 0)
        PANIC_WITH_CTX
}

errcode_t MessageChain::Init() {
    __initialized_with_creds = false;

    if (creat_and_ftok((NODES_SHM_FILE_PREFIX + tag).c_str(), FTOK_PROJ_ID, &nodes_shmkey) > 0)
        PANIC_WITH_CTX

    if (creat_and_ftok((HEAD_INDX_FILE_PREFIX + tag).c_str(), FTOK_PROJ_ID, &head_index_shmkey) > 0)
        PANIC_WITH_CTX

    if (creat_and_ftok((ACT_NODES_FILE_PREFIX + tag).c_str(), FTOK_PROJ_ID, &active_nodes_shmkey) > 0)
        PANIC_WITH_CTX

    void *ptr;

    if (get_or_create_shared_memory(nodes_shmkey, sizeof(data_ptr_t) * TOTAL_CIRCULAR_NODES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    nodes = (data_ptr_t*) ptr;

    if (get_or_create_shared_memory(head_index_shmkey, sizeof(node_index_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    head_index = (node_index_t*) ptr;

    if (get_or_create_shared_memory(active_nodes_shmkey, sizeof(node_index_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    active_nodes = (node_index_t*) ptr;

    *head_index = 0;

    *active_nodes = 0;

    if (rwlock->Init((std::string(tag) + "-rwlock").c_str()) > 0)
        PANIC_WITH_CTX

    return EC_OK;
}

errcode_t MessageChain::InitWithCreds(MCCreds *creds) {
    __initialized_with_creds = true;
    
    void *ptr;

    this->nodes_shmkey = creds->nodes_shmkey;
    this->head_index_shmkey = creds->head_index_shmkey;
    this->active_nodes_shmkey = creds->active_nodes_shmkey;

    if (get_or_create_shared_memory(creds->nodes_shmkey, sizeof(data_ptr_t) * TOTAL_CIRCULAR_NODES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    nodes = (data_ptr_t*) ptr;

    if (get_or_create_shared_memory(creds->head_index_shmkey, sizeof(node_index_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    head_index = (node_index_t*) ptr;

    if (get_or_create_shared_memory(creds->active_nodes_shmkey, sizeof(node_index_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    active_nodes = (node_index_t*) ptr;

    if (rwlock->Init((std::string(tag) + "-rwlock").c_str(), creds->rwcreds) > 0)
        PANIC_WITH_CTX

    return EC_OK;
}

errcode_t MessageChain::nth_node(data_ptr_t *ptr, node_index_t index) {
    if (NULL == ptr)
        TRACE_AND_RETURN(EC_ARG1_IS_NULL)

    *ptr = nodes[index % TOTAL_CIRCULAR_NODES];
    return EC_OK;
}

node_index_t MessageChain::tail_index() {
    node_index_t tail = *head_index + *active_nodes;
    return tail % TOTAL_CIRCULAR_NODES;
}

node_index_t MessageChain::GetTailIndex() {
    rwlock->acquire_wrlock();
    node_index_t tail = this->tail_index();
    rwlock->release_wrlock();
    return tail;
}

errcode_t MessageChain::Push(data_ptr_t ptr) {
    rwlock->acquire_wrlock();

    if (*active_nodes >= TOTAL_CIRCULAR_NODES) {
        rwlock->release_wrlock();    
        TRACE_AND_RETURN(EC_CIRCULAR_COLLISION)
    }

    node_index_t tail_index = this->tail_index();

    nodes[tail_index] = ptr;
    (*active_nodes)++;
    rwlock->release_wrlock();

    return EC_OK;
}

errcode_t MessageChain::Pop(data_ptr_t* ptr) {
    rwlock->acquire_wrlock();

    if (NULL == ptr) {
        rwlock->release_wrlock();
        TRACE_AND_RETURN(EC_ARG1_IS_NULL)
    }

    if (*active_nodes == 0) {
        rwlock->release_wrlock();
        TRACE_AND_RETURN(EC_EMPTY_CHAIN)
    }

    errcode_t ec = this->nth_node(ptr, *head_index);
     
    if (ec > 0) {
        rwlock->release_wrlock();
        PANIC_WITH_CTX
    }

    *head_index = (*head_index + 1) % TOTAL_CIRCULAR_NODES;
    (*active_nodes)--;

    rwlock->release_wrlock();
    return EC_OK;
}

std::string MessageChain::Dump() {
    std::ostringstream stream;
    
    stream << "[";
    bool first_elem = true;

    rwlock->acquire_wrlock();

    for (node_index_t i = 0; i < *active_nodes; i++) {
        if (first_elem)
            first_elem = false;
        else if (*active_nodes > 1)
            stream << ", ";

        stream << nodes[(*head_index + i) % TOTAL_CIRCULAR_NODES];
    }

    rwlock->release_wrlock();

    stream << "]";

    return stream.str(); 
}

MCCreds* MessageChain::GetCreds() {
    MCCreds *c = new MCCreds();

    c->active_nodes_shmkey = this->active_nodes_shmkey;
    c->head_index_shmkey = this->head_index_shmkey;
    c->nodes_shmkey = this->nodes_shmkey;
    c->rwcreds = this->rwlock->GetCreds();

    return c;
}