#ifndef __RWLOCK_H
#define __RWLOCK_H

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <deque>
#include <mutex>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include "common.h"

typedef struct _rwcreds {
    const char  *lock_sem_file;
    const char  *wrlock_sem_file;
    key_t       readers_shmkey;
} RWCreds;

typedef struct _rwlock {
    // RWLock();
    errcode_t Init(const char *tag);
    errcode_t Init(const char *tag, RWCreds *creds);
    errcode_t Clear();

    void acquire_rdlock();
    void acquire_wrlock();
    void release_rdlock();
    void release_wrlock();

    RWCreds* GetCreds();

private:
    std::string tag;
    std::string lock_sem_file;
    std::string wrlock_sem_file;
    sem_t *lock;
    sem_t *write_lock;
    key_t readers_shmkey;
    int *readers;
} RWLock;

#endif