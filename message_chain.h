#ifndef __MESSAGE_CHAIN_H
#define __MESSAGE_CHAIN_H

#include "common.h"

typedef void* data_ptr_t; 
typedef uint16_t node_index_t; 

class MessageChain {
public:
    MessageChain();    
    ~MessageChain();    
    errcode_t Push(data_ptr_t);
    errcode_t Pop(data_ptr_t*);

private:
    data_ptr_t nodes[TOTAL_CIRCULAR_NODES];
    node_index_t head_index;
    node_index_t active_nodes;
    errcode_t nth_node(data_ptr_t*, node_index_t);
    node_index_t tail_index();
};

#endif
