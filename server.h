#ifndef __SERVER_H
#define __SERVER_H

#include "common.h"

class Server {
public:
    Server(const char* sock_path);
    errcode_t Bind();
    errcode_t Listen();
    errcode_t AllocateSharedMemory();
    errcode_t PreserveSynchronization();
    errcode_t Run();

private:
    int sock_fd;
    std::string sock_path;
};

#endif
