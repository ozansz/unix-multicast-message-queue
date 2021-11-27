#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#define TOTAL_ACTIVE_MESSAGES   1024
#define TOTAL_CIRCULAR_NODES    TOTAL_ACTIVE_MESSAGES
#define MESSAGE_LENGTH_BYTES    512
#define TOTAL_ACTIVE_CLIENTS    100

#define SHARED_MEMORY_FILE      "536-shm"
#define SHARED_MEMORY_PROJ_ID   'Q'

#define LOCK_SEM_PREFIX                     "./__lock_sem-"
#define WRLOCK_SEM_PREFIX                   "./__wrlock_sem-"
#define READERS_SHM_FILE_PREFIX             "./__readers-"
#define FTOK_PROJ_ID                        'R'
#define NODES_SHM_FILE_PREFIX               "./__nodes-"
#define HEAD_INDX_FILE_PREFIX               "./__head_indx-"
#define ACT_NODES_FILE_PREFIX               "./__act_nodes-"
#define Q_BLOCK_SHM_FILE_PREFIX             "./__queue-block"
#define Q_MSG_RDCNT_SHM_FILE_PREFIX         "./__queue-msg-rdcnt"
#define Q_ACT_CLNTS_SHM_FILE_PREFIX         "./__queue-act-clnts"
#define Q_CLI_CHAINS_SHM_FILE_PREFIX        "./__queue-cli-chains"
#define Q_CLI_LIVENESSMAP_SHM_FILE_PREFIX   "./__queue-cli-liveness"
#define Q_CLI_REFMAP_SHM_FILE_PREFIX        "./__queue-cli-refmap"

#endif
