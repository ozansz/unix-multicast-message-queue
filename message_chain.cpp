#include "message_chain.h"

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
    std::cout << "i1\n";
    if (creat_and_ftok((NODES_SHM_FILE_PREFIX + tag).c_str(), FTOK_PROJ_ID, &nodes_shmkey) > 0)
        PANIC_WITH_CTX
    std::cout << "i2\n";

    if (creat_and_ftok((HEAD_INDX_FILE_PREFIX + tag).c_str(), FTOK_PROJ_ID, &head_index_shmkey) > 0)
        PANIC_WITH_CTX
    std::cout << "i3\n";

    if (creat_and_ftok((ACT_NODES_FILE_PREFIX + tag).c_str(), FTOK_PROJ_ID, &active_nodes_shmkey) > 0)
        PANIC_WITH_CTX
    std::cout << "i4\n";

    void *ptr;
    std::cout << "i5\n";

    if (get_or_create_shared_memory(nodes_shmkey, sizeof(data_ptr_t) * TOTAL_CIRCULAR_NODES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX
    std::cout << "i6\n";

    nodes = (data_ptr_t*) ptr;
    std::cout << "i7\n";

    if (get_or_create_shared_memory(head_index_shmkey, sizeof(node_index_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX
    std::cout << "i8\n";

    head_index = (node_index_t*) ptr;
    std::cout << "i9\n";

    if (get_or_create_shared_memory(active_nodes_shmkey, sizeof(node_index_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX
    std::cout << "i10\n";

    active_nodes = (node_index_t*) ptr;
    std::cout << "i11\n";

    *head_index = 0;
    std::cout << "i12\n";

    *active_nodes = 0;
    std::cout << "i13\n";

    if (rwlock->Init((std::string(tag) + "-rwlock").c_str()) > 0)
        PANIC_WITH_CTX
    std::cout << "i14\n";

    return EC_OK;
}

errcode_t MessageChain::InitWithCreds(MCCreds *creds) {
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

errcode_t MessageChain::Push(data_ptr_t ptr) {
    std::cout << "MC-P1\n";
    rwlock->acquire_wrlock();
    std::cout << "MC-P2\n";

    if (*active_nodes >= TOTAL_CIRCULAR_NODES) {
    std::cout << "MC-P3\n";

        rwlock->release_wrlock();
    std::cout << "MC-P4\n";

        TRACE_AND_RETURN(EC_CIRCULAR_COLLISION)
    }
    std::cout << "MC-P5\n";

    node_index_t tail_index = this->tail_index();
    std::cout << "MC-P6\n";
    std::cout << ptr << "\n";
    std::cout << nodes << "\n";
    std::cout << tail_index << "\n";
    std::cout << nodes + tail_index << "\n";
    std::cout << *nodes << "\n";
    std::cout << *(nodes+tail_index) << "\n";

    nodes[tail_index] = ptr;
    std::cout << "MC-P7\n";

    (*active_nodes)++;
    std::cout << "MC-P8\n";


    rwlock->release_wrlock();
    std::cout << "MC-P9\n";

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