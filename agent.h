#ifndef __AGENT_H
#define __AGENT_H

#include "common.h"

class Agent {
public:
    Agent(int client_sock);
    errcode_t Run();

private:
    int client_sock;
};

#endif