#ifndef __MESSAGE_QUEUE_H
#define __MESSAGE_QUEUE_H

#include "common.h"
#include "message_chain.h"
#include "rwlock.h"

typedef struct _mqcreds {
    key_t queue_block_shmkey;
    key_t queue_refs_shmkey;
    key_t active_clients_shmkey;
    key_t client_chains_shmkey;

    key_t client_liveness_map_shmkey;
    key_t client_references_map_shmkey;
    
    RWCreds *queue_block_rwlock_creds;
    MCCreds *queue_block_mc_creds;
    MCCreds *client_mc_creds[TOTAL_ACTIVE_CLIENTS];

    std::string Dump();
} MQCreds;

class MessageQueue {
public:
    MessageQueue();
    ~MessageQueue();

    errcode_t Init();
    errcode_t InitWithCreds(MQCreds* creds);
    MQCreds* GetCreds();
    errcode_t PopImmediate(/* uint16_t index, */ char *data, bool *could_pop, uint16_t client_id);
    // errcode_t PopWait(std::string *data, uint16_t client_id);
    errcode_t Push(std::string data, uint16_t *index /*, uint16_t client_id */);
    // errcode_t SetActiveClients(uint16_t num);
    errcode_t MakeClientAliveWithID(uint16_t client_id);
    errcode_t InvalidateClientWithID(uint16_t client_id);
    std::string Dump();

private:
    bool __initialized_with_creds;

    MessageChain *chain;
    char *queue_block;
    key_t queue_block_shmkey;
    RWLock *rwlock;

    // uint16_t refs[TOTAL_ACTIVE_MESSAGES];
    uint16_t *refs;
    key_t queue_refs_shmkey;

    // uint16_t active_clients;
    uint16_t *active_clients;
    key_t active_clients_shmkey;

    // bool client_liveness_map[TOTAL_ACTIVE_CLIENTS];
    bool *client_liveness_map;
    key_t client_liveness_map_shmkey;

    // bool client_references_map[TOTAL_ACTIVE_CLIENTS][TOTAL_ACTIVE_MESSAGES];
    bool *client_references_map;
    key_t client_references_map_shmkey;

    // MessageChain *client_chains[100]; (TOTAL_ACTIVE_CLIENTS = 100)
    MessageChain **client_chains;
    key_t client_chains_shmkey;
};

#endif
