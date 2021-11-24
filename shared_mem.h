#ifndef __SHARED_MEM_H
#define __SHARED_MEM_H

#include "common.h"

const size_t shared_mem_item_size = MESSAGE_LENGTH_BYTES;
const size_t shared_mem_total_size = TOTAL_ACTIVE_MESSAGES * shared_mem_item_size;

#endif