#include "shared_mem.h"
#include "message_chain.h"

int __shmid = 0;
char *___shm_data_ptr = NULL;
key_t __shared_memory_key;
MessageChain *shared_memory_chain;

std::mutex shm_read_mutex;

errcode_t init_shared_memory() {
    __shared_memory_key = ftok(SHARED_MEMORY_FILE, SHARED_MEMORY_PROJ_ID);

    if (__shared_memory_key < 0) {
        TRACE_WITH_ERRNO_AND_RETURN(EC_FTOK)
    }

    __shmid = shmget(__shared_memory_key, shared_mem_total_size, 0666 | IPC_CREAT);

    if (__shmid < 0) {
        TRACE_WITH_ERRNO_AND_RETURN(EC_SHMGET)
    }

    ___shm_data_ptr = shmat(__shmid, NULL, 0);

    if (___shm_data_ptr <= 0) {
        TRACE_AND_RETURN(EC_SHMAT)
    }

    shared_memory_chain = new MessageChain();

    if (NULL == shared_memory_chain) {
        TRACE_AND_RETURN(EC_MC_ALLOC)
    }

    return EC_OK;
}

errcode_t get_shared_memory_block_with_index(, size_t index) {
    // const std::lock_guard<std::mutex> lock(shm_read_mutex);

    // if (0 == __shmid) {
    //     TRACE_AND_RETURN(EC_SHM_UNINITIALIZED)
    // }

    // ___shm_data_ptr
    
    // TODO : Implement here!
}