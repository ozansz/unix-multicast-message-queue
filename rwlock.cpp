#include "rwlock.h"

errcode_t RWLock::Init(const char* _tag) {
    this->tag = _tag;

    std::cout << "r1\n";

    lock_sem_file = LOCK_SEM_PREFIX + tag;
    wrlock_sem_file = WRLOCK_SEM_PREFIX + tag;
std::cout << "r2\n";
    lock = sem_open(lock_sem_file.c_str(), O_CREAT, 0644, 1);
std::cout << "r3\n";
    if (lock == SEM_FAILED)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_OPEN)
std::cout << "r4\n";
    write_lock = sem_open(wrlock_sem_file.c_str(), O_CREAT, 0644, 1);
std::cout << "r5\n";
    if (write_lock == SEM_FAILED)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_OPEN)
std::cout << "r6\n";
    if (creat_and_ftok((READERS_SHM_FILE_PREFIX + tag).c_str(), FTOK_PROJ_ID, &readers_shmkey) > 0)
        PANIC_WITH_CTX
std::cout << "r7\n";
    void *ptr;
std::cout << "r8\n";
    if (get_or_create_shared_memory(readers_shmkey, sizeof(int), IPC_CREAT | 0666, &ptr) > 0)
        PANIC_WITH_CTX
std::cout << "r9\n";
    readers = (int*) ptr;
    std::cout << "r10\n";
    *readers = 0;
std::cout << "r11\n";
    return EC_OK;
}

errcode_t RWLock::Init(const char* _tag, RWCreds *creds) {
    this->tag = _tag;

    this->lock_sem_file = creds->lock_sem_file;
    this->wrlock_sem_file = creds->wrlock_sem_file;
    this->readers_shmkey = creds->readers_shmkey;
    
    lock = sem_open(lock_sem_file.c_str(), 0);

    if (lock == SEM_FAILED)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_OPEN)

    write_lock = sem_open(wrlock_sem_file.c_str(), 0);

    if (write_lock == SEM_FAILED)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_OPEN)

    key_t shmid = shmget(readers_shmkey, sizeof(int), IPC_CREAT | 0666);

    if (shmid < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SHMGET)

    readers = (int*) shmat(shmid, NULL, 0);

    if (readers == NULL)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SHMAT)

    return EC_OK;
}


errcode_t RWLock::Clear() {
    if (sem_close(lock) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_CLOSE)

    if (sem_close(write_lock) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_CLOSE)

    if (sem_unlink(lock_sem_file.c_str()) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_UNLINK)

    if (sem_unlink(wrlock_sem_file.c_str()) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEM_UNLINK)

    // if (munmap(readers, sizeof(int)) < 0)
    //     TRACE_WITH_ERRNO_AND_RETURN(EC_MUNMAP)

    if (shmdt(readers) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SHMDT)

    return EC_OK;
}

void RWLock::acquire_rdlock() {
    sem_wait(lock);
    (*readers)++;
    if (*readers == 1)
        sem_wait(write_lock);
    sem_post(lock);
}

void RWLock::release_rdlock() {
    sem_wait(lock);
    (*readers)--;
    if (*readers == 0)
        sem_post(write_lock);
    sem_post(lock);
}

void RWLock::acquire_wrlock() {
    sem_wait(write_lock);
}

void RWLock::release_wrlock() {
    sem_post(write_lock);
}

RWCreds* RWLock::GetCreds() {
    RWCreds *c = new RWCreds();

    c->lock_sem_file = this->lock_sem_file.c_str();
    c->wrlock_sem_file = this->wrlock_sem_file.c_str();
    c->readers_shmkey = this->readers_shmkey;

    return c;
}
