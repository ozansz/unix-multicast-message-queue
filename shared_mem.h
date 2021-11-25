#ifndef __SHARED_MEM_H
#define __SHARED_MEM_H

#include "common.h"

errcode_t get_or_create_shared_memory(key_t shmkey, size_t size, int prot, void **ptr);
errcode_t creat_and_ftok(const char* path, int proj_id, key_t *key);

#endif