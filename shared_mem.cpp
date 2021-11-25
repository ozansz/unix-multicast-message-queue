#include "shared_mem.h"
#include "message_chain.h"

errcode_t get_or_create_shared_memory(key_t shmkey, size_t size, int prot, void **ptr) {
    key_t shmid = shmget(shmkey, size, prot);

    if (shmid < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SHMGET)

    *ptr = shmat(shmid, NULL, 0);

    if (*ptr == NULL)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SHMAT)

    return EC_OK;
}

errcode_t creat_and_ftok(const char* path, int proj_id, key_t *key) {
    int fd = open(path, O_CREAT | O_RDWR, 0777);

    if (fd < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_OPEN)

    if (close(fd) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_CLOSE)

    key_t shmkey = ftok(path, proj_id);

    if (shmkey < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_FTOK)

    *key = shmkey;

    return EC_OK;
}
