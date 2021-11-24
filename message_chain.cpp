#include "message_chain.h"

MessageChain::MessageChain() {
    this->head_index = 0;
    this->active_nodes = 0;
    
    for (node_index_t i = 0; i < TOTAL_CIRCULAR_NODES; i++)
        this->nodes[i] = 0;
}

errcode_t MessageChain::nth_node(data_ptr_t *ptr, node_index_t index) {
    if (NULL == ptr)
        TRACE_AND_RETURN(EC_ARG1_IS_NULL)

    *ptr = this->nodes[index % TOTAL_CIRCULAR_NODES];
    return EC_OK;
}

node_index_t MessageChain::tail_index() {
    node_index_t tail = this->head_index + this->active_nodes;
    return tail % TOTAL_CIRCULAR_NODES;
}

errcode_t MessageChain::Push(data_ptr_t ptr) {
    if (this->active_nodes >= TOTAL_CIRCULAR_NODES)
        TRACE_AND_RETURN(EC_CIRCULAR_COLLISION)

    node_index_t tail_index = this->tail_index();
    this->nodes[tail_index] = ptr;
    this->active_nodes++;

    return EC_OK;
}

errcode_t MessageChain::Pop(data_ptr_t* ptr) {
    if (NULL == ptr)
        TRACE_AND_RETURN(EC_ARG1_IS_NULL)

    if (this->active_nodes == 0)
        TRACE_AND_RETURN(EC_EMPTY_CHAIN)

    errcode_t ec = this->nth_node(ptr, this->head_index);
     
    if (ec > 0) {
        PANIC_WITH_CTX
    }

    this->head_index = (this->head_index + 1) % TOTAL_CIRCULAR_NODES;
    this->active_nodes--;

    return EC_OK;
}