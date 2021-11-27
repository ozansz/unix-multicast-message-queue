#include "message_queue.h"

bool ___glob_message_queue_init = false;

std::string self_tag = "main-queue";

std::string MQCreds::Dump() {
    std::ostringstream stream;

    stream << "queue_block_shmkey: " << queue_block_shmkey << "\n";
    stream << "queue_refs_shmkey: " << queue_refs_shmkey << "\n";
    stream << "active_clients_shmkey: " << active_clients_shmkey << "\n";
    stream << "client_chains_shmkey: " << client_chains_shmkey << "\n";
    stream << "client_liveness_map_shmkey: " << client_liveness_map_shmkey << "\n";
    stream << "client_references_map_shmkey: " << client_references_map_shmkey << "\n";

    stream << "\nrwlock: \n" << queue_block_rwlock_creds->Dump();
    stream << "\nchain: \n" << queue_block_mc_creds->Dump();

    stream << "\nclient chains: \n";
    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++)
        stream << "client-" << i << ": \n" << client_mc_creds[i]->Dump() << "\n\n"; 

    return stream.str();
}

MessageQueue::MessageQueue() {
    // This assert may be erroneous. Check later!
    assert(!___glob_message_queue_init);

    chain = new MessageChain((self_tag + "-chain").c_str());
    rwlock = new RWLock();

    ___glob_message_queue_init = true;
}

MessageQueue::~MessageQueue() {
    if (shmdt(queue_block) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (shmdt(refs) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (shmdt(active_clients) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (shmdt(client_liveness_map) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (shmdt(client_references_map) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    if (rwlock->Clear() > 0)
        PANIC_WITH_CTX

    // NOTE: Not sure if i should do this delete here.
    // Check if this closes the other clients'
    //connections too.
    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++)
        delete client_chains[i];

    if (shmdt(client_chains) < 0) {
        TRACE_APPEND_WITH_CTX(EC_SHMDT, ERRNO_STR)
        PANIC_WITH_CTX
    }

    delete chain;
}

errcode_t MessageQueue::Init() {
    __initialized_with_creds = false;

    if (creat_and_ftok(Q_BLOCK_SHM_FILE_PREFIX, FTOK_PROJ_ID, &queue_block_shmkey) > 0)
        PANIC_WITH_CTX

    if (creat_and_ftok(Q_MSG_RDCNT_SHM_FILE_PREFIX, FTOK_PROJ_ID, &queue_refs_shmkey) > 0)
        PANIC_WITH_CTX

    if (creat_and_ftok(Q_ACT_CLNTS_SHM_FILE_PREFIX, FTOK_PROJ_ID, &active_clients_shmkey) > 0)
        PANIC_WITH_CTX

    if (creat_and_ftok(Q_CLI_CHAINS_SHM_FILE_PREFIX, FTOK_PROJ_ID, &client_chains_shmkey) > 0)
        PANIC_WITH_CTX

    if (creat_and_ftok(Q_CLI_LIVENESSMAP_SHM_FILE_PREFIX, FTOK_PROJ_ID, &client_liveness_map_shmkey) > 0)
        PANIC_WITH_CTX

    if (creat_and_ftok(Q_CLI_REFMAP_SHM_FILE_PREFIX, FTOK_PROJ_ID, &client_references_map_shmkey) > 0)
        PANIC_WITH_CTX

    void *ptr;

    if (get_or_create_shared_memory(queue_block_shmkey, sizeof(char) * MESSAGE_LENGTH_BYTES * TOTAL_ACTIVE_MESSAGES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    queue_block = (char*) ptr;

    if (get_or_create_shared_memory(queue_refs_shmkey, sizeof(uint16_t) * TOTAL_ACTIVE_MESSAGES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    refs = (uint16_t*) ptr;

    if (get_or_create_shared_memory(active_clients_shmkey, sizeof(uint16_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    active_clients = (uint16_t*) ptr;

    if (get_or_create_shared_memory(client_chains_shmkey, sizeof(MessageChain*) * TOTAL_ACTIVE_CLIENTS, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    client_chains = (MessageChain**) ptr;

    if (get_or_create_shared_memory(client_liveness_map_shmkey, sizeof(bool) * TOTAL_ACTIVE_CLIENTS, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    client_liveness_map = (bool*) ptr;

    if (get_or_create_shared_memory(client_references_map_shmkey, sizeof(bool) * TOTAL_ACTIVE_CLIENTS * TOTAL_ACTIVE_MESSAGES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    client_references_map = (bool*) ptr;

    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++) {
        client_liveness_map[i] = false;
        
        for (uint16_t j = 0; j < TOTAL_ACTIVE_MESSAGES; j++)
            *(client_references_map + (i * TOTAL_ACTIVE_MESSAGES) + j) = false;
    }

    // ADDED:   client_liveness_map and client_references_map
    //
    // TODO:    ~~initialize client_liveness_map and client_references_map here and in InitWithCreds()~~
    //          implement usage cases for client_liveness_map and client_references_map

    *active_clients = 0;

    for (uint16_t i = 0; i < TOTAL_ACTIVE_MESSAGES; i++)
        memset(queue_block + (i * MESSAGE_LENGTH_BYTES), 0, MESSAGE_LENGTH_BYTES);

    for (uint16_t i = 0; i < TOTAL_ACTIVE_MESSAGES; i++)
        refs[i] = 0;
 
    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++) {
        client_chains[i] = new MessageChain(("mc-cli-" + std::to_string(i)).c_str());
        if (client_chains[i]->Init() > 0)
            PANIC_WITH_CTX
    }

    if (rwlock->Init((self_tag + "-lock").c_str()) > 0)
        PANIC_WITH_CTX

    if (chain->Init() > 0)
        PANIC_WITH_CTX

    return EC_OK;
}

errcode_t MessageQueue::InitWithCreds(MQCreds* creds) {
    __initialized_with_creds = true;

    this->active_clients_shmkey = creds->active_clients_shmkey;
    this->queue_block_shmkey = creds->queue_block_shmkey;
    this->queue_refs_shmkey = creds->queue_refs_shmkey;
    this->client_chains_shmkey = creds->client_chains_shmkey;
    this->client_liveness_map_shmkey = creds->client_liveness_map_shmkey;
    this->client_references_map_shmkey = creds->client_references_map_shmkey;

    void *ptr;

    if (get_or_create_shared_memory(queue_block_shmkey, sizeof(char) * MESSAGE_LENGTH_BYTES * TOTAL_ACTIVE_MESSAGES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    queue_block = (char*) ptr;

    if (get_or_create_shared_memory(queue_refs_shmkey, sizeof(uint16_t) * TOTAL_ACTIVE_MESSAGES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    refs = (uint16_t*) ptr;

    if (get_or_create_shared_memory(active_clients_shmkey, sizeof(uint16_t), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    active_clients = (uint16_t*) ptr;

    if (get_or_create_shared_memory(client_chains_shmkey, sizeof(MessageChain*) * TOTAL_ACTIVE_CLIENTS, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    client_chains = (MessageChain**) ptr;

    if (get_or_create_shared_memory(client_liveness_map_shmkey, sizeof(bool) * TOTAL_ACTIVE_CLIENTS, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    client_liveness_map = (bool*) ptr;

    if (get_or_create_shared_memory(client_references_map_shmkey, sizeof(bool) * TOTAL_ACTIVE_CLIENTS * TOTAL_ACTIVE_MESSAGES, IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX

    client_references_map = (bool*) ptr;

    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++) {
        client_chains[i] = new MessageChain(("mc-cli-" + std::to_string(i)).c_str());
        if (client_chains[i]->InitWithCreds(creds->client_mc_creds[i]) > 0)
            PANIC_WITH_CTX
    }

    if (chain->InitWithCreds(creds->queue_block_mc_creds) > 0)
        PANIC_WITH_CTX

    if (rwlock->Init((self_tag + "-lock").c_str(), creds->queue_block_rwlock_creds) > 0)
        PANIC_WITH_CTX

    // DESIGN NOTE:
    //
    //   - Should we increment the active client count in here or
    // in the agent code right after the fork() of the client
    // agent?  

    return EC_OK;
}

MQCreds* MessageQueue::GetCreds() {
    MQCreds *c = new MQCreds();

    c->active_clients_shmkey = this->active_clients_shmkey;
    c->queue_block_shmkey = this->queue_block_shmkey;
    c->queue_refs_shmkey = this->queue_refs_shmkey;
    c->client_liveness_map_shmkey = this->client_liveness_map_shmkey;
    c->client_references_map_shmkey = this->client_references_map_shmkey;
    c->queue_block_rwlock_creds = this->rwlock->GetCreds();
    c->queue_block_mc_creds = this->chain->GetCreds();

    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++)
        c->client_mc_creds[i] = this->client_chains[i]->GetCreds();

    return c;
}

errcode_t MessageQueue::PopImmediate(/* uint16_t index, */ char *data, bool *could_pop, uint16_t client_id) {
    this->rwlock->acquire_wrlock();

    uint16_t index;
    errcode_t ec = client_chains[client_id]->Pop(&index);

    if (ec > 0) {
        if (ec == EC_EMPTY_CHAIN) {
            *could_pop = false;
            return EC_OK;
        } else {
            this->rwlock->release_wrlock();
            PANIC_WITH_CTX
        }
    }

    *could_pop = true;
    data = new char[MESSAGE_LENGTH_BYTES];

    memcpy(data, queue_block + (index * MESSAGE_LENGTH_BYTES), MESSAGE_LENGTH_BYTES);

    if (refs[index] < 1) {
        this->rwlock->release_wrlock();
        TRACE_WITH_CTX_AND_RETURN(EC_TRY_DEC_ZERO_REF, ("client-" + std::to_string(client_id)).c_str())
    }

    *(client_references_map + (client_id * TOTAL_ACTIVE_MESSAGES) + index) = false;

    refs[index]--;

    if (refs[index] == 0) {
        // Invalidate the data from shared queue block
        
        uint16_t chain_head_data;
        // This Pop() call SHALL return the same "index".
        // If not, we have poorly designed the whole
        // synchronized shared queue and we shall burn in
        // the hell of Windows!
        if (chain->Pop(&chain_head_data) > 0) {
            this->rwlock->release_wrlock();
            PANIC_WITH_CTX
        }

        if (chain_head_data != index) {
            this->rwlock->release_wrlock();
            TRACE_WITH_CTX_AND_RETURN(EC_SHARED_INDEX_SYNC, ("index = " + std::to_string(index) + ", chain_head_data = " + std::to_string(chain_head_data)).c_str())
        }

        // Invalidation should be done by now, I don't think
        //   there is more we need to do... I hope :)
    }

    this->rwlock->release_wrlock();
    return EC_OK;
}


// errcode_t MessageQueue::PopWait(std::string *data, uint16_t client_id) {

// }

errcode_t MessageQueue::Push(std::string data, uint16_t *index /*, uint16_t client_id */) {
    this->rwlock->acquire_wrlock();

    *index = chain->GetTailIndex();
    chain->Push(*index);
    refs[*index] = *active_clients;
    
    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++)
        if (client_liveness_map[i])
            *(client_references_map + (i * TOTAL_ACTIVE_MESSAGES) + (*index)) = true;

    memcpy(queue_block + ((*index) * MESSAGE_LENGTH_BYTES), data.c_str(), MESSAGE_LENGTH_BYTES);

    for (uint16_t i = 0; i < TOTAL_ACTIVE_CLIENTS; i++)
        if (client_liveness_map[i])                 // ONLY push the index to client chain if it's alive
                client_chains[i]->Push(*index);     // ELSE, it will possibly overwrite the client chain with un-popped >1024 messages...    

    this->rwlock->release_wrlock();
    return EC_OK;
}

errcode_t MessageQueue::MakeClientAliveWithID(uint16_t client_id) {
    this->rwlock->acquire_wrlock();

    *active_clients = *active_clients + 1;

    client_liveness_map[client_id] = true;

    this->rwlock->release_wrlock();
    return EC_OK;
}

errcode_t MessageQueue::InvalidateClientWithID(uint16_t client_id) {
    this->rwlock->acquire_wrlock();

    for (uint16_t index = 0; index < TOTAL_ACTIVE_MESSAGES; index++)
        if(true == *(client_references_map + (client_id * TOTAL_ACTIVE_MESSAGES) + index)) {
            if (refs[index] < 1) {
                this->rwlock->release_wrlock();
                TRACE_WITH_CTX_AND_RETURN(EC_TRY_DEC_ZERO_REF, ("client-" + std::to_string(client_id)).c_str())
            } else
                refs[index]--;
        }

    *active_clients = *active_clients - 1;

    client_liveness_map[client_id] = false;

    this->rwlock->release_wrlock();
    return EC_OK;
}

// errcode_t MessageQueue::SetActiveClients(uint16_t num) {
//     this->rwlock->acquire_wrlock();

//     *active_clients = num;

//     this->rwlock->release_wrlock();
//     return EC_OK;
// }

std::string MessageQueue::Dump() {
    std::ostringstream stream;
    
    // rwlock->acquire_rdlock();
    rwlock->acquire_wrlock();

    stream << "active: " << *active_clients << "\n\n";
    stream << "refs: {";

    for (uint16_t i = 0; i < TOTAL_ACTIVE_MESSAGES; i++)
        if (refs[i] > 0)
            stream << "    " << i << ": " << refs[i] << ",\n";

    stream << "}\n\n";

    stream << "refmap: {";
    
    for (uint16_t client_id = 0; client_id < TOTAL_ACTIVE_CLIENTS; client_id++)
        if (client_liveness_map[client_id]) {
            stream << "    " << client_id << ": [";
            
            for (uint16_t index = 0; index < TOTAL_ACTIVE_MESSAGES; index++) 
                if(true == *(client_references_map + (client_id * TOTAL_ACTIVE_MESSAGES) + index))
                    stream << " " << index;

            stream << " ],\n";
        }

    stream << "}\n\n";

    stream << chain->Dump() << "\n";
    
    for (uint16_t client_id = 0; client_id < TOTAL_ACTIVE_CLIENTS; client_id++)
        if (client_liveness_map[client_id])
            stream << "\n" << client_chains[client_id]->Dump();

    stream << "\n";

    // rwlock->release_rdlock();
    rwlock->release_wrlock();

    return stream.str(); 
}
