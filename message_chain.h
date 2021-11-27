#ifndef __MESSAGE_CHAIN_H
#define __MESSAGE_CHAIN_H

#include "common.h"

// #define ALLOW_CHAIN_COLLISIONS  1

// typedef void* data_ptr_t; 
typedef uint16_t data_ptr_t; 
typedef uint16_t node_index_t; 

typedef struct _mccreds {
    key_t nodes_shmkey;
    key_t head_index_shmkey;
    key_t active_nodes_shmkey;

    RWCreds *rwcreds;

    std::string Dump();
} MCCreds;

class MessageChain {
public:
    MessageChain(const char *tag);    
    ~MessageChain();    
    errcode_t Init();
    errcode_t InitWithCreds(MCCreds *creds);
    errcode_t Push(data_ptr_t);
    errcode_t Pop(data_ptr_t*);
    std::string Dump();
    MCCreds* GetCreds();
    node_index_t GetTailIndex();

private:
    bool __initialized_with_creds;

    std::string tag;

    RWLock *rwlock;
    
    //data_ptr_t nodes[TOTAL_CIRCULAR_NODES];
    key_t nodes_shmkey;
    data_ptr_t *nodes;
    
    key_t head_index_shmkey;
    node_index_t *head_index;
    key_t active_nodes_shmkey;
    node_index_t *active_nodes;

    errcode_t nth_node(data_ptr_t*, node_index_t);
    node_index_t tail_index();
};

#endif
