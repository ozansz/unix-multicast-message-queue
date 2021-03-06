#include "errors.h"

#include <cstdio>
#include <string>
#include <errno.h>

const char* err_codes_map[] = {
    "OK",
    "Runtime error (unknown)",
    "Function argument #1 is NULL",
    "Function argument #2 is NULL",
    "Function argument #3 is NULL",
    "Collision in MessageChain (write on existing)",
    "MessageChain is empty",
    "socket() has failed",
    "bind() has failed",
    "close() has failed",
    "listen() has failed",
    "accept() has failed",
    "unlink() has failed",
    "Server is uninitialized",
    "ftok() has failed",
    "Could not allocate new MessageChain",
    "shmget() has failed",
    "Shared memory is uninitialized",
    "shmat() has failed",
    "sem_open() has failed",
    "sem_close() has failed",
    "sem_unlink() has failed",
    "munmap() has failed",
    "shmdt() has failed",
    "creat() has failed",
    "open() has failed",
    "Tried do decrement refs[index] which was already zero",
    "Shared data index synchronization error",
};

const char* get_err_desc(errcode_t code) {
    if (code > (sizeof(err_codes_map) / sizeof(err_codes_map[0])))
        return NULL;

    return err_codes_map[code];
}