#include "agent.h"

Agent::Agent(uint16_t client_id, int client_sock, MessageQueue *queue) {
    this->client_id = client_id;
    this->client_sock = client_sock;
    this->queue = queue;
}

errcode_t Agent::Run() {
    const MCCreds *creds = this->queue->GetCreds();
    if (send(this->client_sock, creds, sizeof(creds)) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SEND)

    if (this->EventLoop() > 0)
        PANIC_WITH_CTX

    return EC_OK;
}