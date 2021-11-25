#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define TOTAL_ACTIVE_MESSAGES   1024
#define TOTAL_CIRCULAR_NODES    TOTAL_ACTIVE_MESSAGES
#define MESSAGE_LENGTH_BYTES    512
#define TOTAL_ACTIVE_CLIENTS    100

#define SHARED_MEMORY_FILE      "536-shm"
#define SHARED_MEMORY_PROJ_ID   'Q'

#define LOCK_SEM_PREFIX         "./__lock_sem-"
#define WRLOCK_SEM_PREFIX       "./__wrlock_sem-"
#define READERS_SHM_FILE_PREFIX "./__readers-"
#define FTOK_PROJ_ID            'R'
#define NODES_SHM_FILE_PREFIX   "./__nodes-"
#define HEAD_INDX_FILE_PREFIX   "./__head_indx-"
#define ACT_NODES_FILE_PREFIX   "./__act_nodes-"

// #define SHMKEY_BASE             0x1000
// #define GET_SHMKEY()

#endif
