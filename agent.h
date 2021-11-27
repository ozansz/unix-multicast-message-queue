#ifndef __AGENT_H
#define __AGENT_H

#include "common.h"
#include "message_queue.h"

class Agent {
public:
    Agent(uint16_t client_id, int client_sock, MessageQueue *queue);
    errcode_t Run();

private:
    uint16_t client_id;
    int client_sock;
    MessageQueue *queue;
};

#endif