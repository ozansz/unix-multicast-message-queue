#ifndef __ERRORS_H
#define __ERRORS_H

#include <cstdint>

#define ERRNO_STR (std::string(strerror(errno)) + " (" + std::to_string(errno) + ")").c_str()

// typedef uint64_t errcode_t;

typedef enum {
    EC_OK = 0,
    EC_RUNTIME,              
    EC_ARG1_IS_NULL,         
    EC_ARG2_IS_NULL,         
    EC_ARG3_IS_NULL,         
    EC_CIRCULAR_COLLISION,   
    EC_EMPTY_CHAIN,     
    EC_SOCKET, 
    EC_BIND, 
    EC_CLOSE, 
    EC_LISTEN, 
    EC_ACCEPT, 
    EC_UNLINK, 
    EC_SERVER_UNINITIALIZED,
    EC_FTOK,
    EC_MC_ALLOC,
    EC_SHMGET,
    EC_SHM_UNINITIALIZED,
    EC_SHMAT,
    EC_SEM_OPEN,
    EC_SEM_CLOSE,
    EC_SEM_UNLINK,
    EC_MUNMAP,
    EC_SHMDT,
    EC_CREAT,
    EC_OPEN,
    EC_TRY_DEC_ZERO_REF,
    EC_SHARED_INDEX_SYNC,
} errcode_t;

const char* get_err_desc(errcode_t code);
const char* errno_str();

#endif