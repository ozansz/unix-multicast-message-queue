#include "server.h"
#include "agent.h"
#include "message_queue.h"

Server::Server(const char* sock_path) {
    this->sock_fd = -1;
    this->sock_path = sock_path;
}

errcode_t Server::Bind() {
    struct sockaddr_un server_sockaddr;

    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));             

    this->sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (this->sock_fd < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_SOCKET)

    server_sockaddr.sun_family = AF_UNIX;   
    strncpy(server_sockaddr.sun_path, this->sock_path.c_str(), sizeof(server_sockaddr.sun_path));

    if (bind(this->sock_fd, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr)) < 0) {
        TRACE_WITH_ERRNO_AND_RETURN(EC_BIND)
        
        if (close(this->sock_fd) < 0) {
            TRACE_WITH_ERRNO_AND_RETURN(EC_CLOSE)
        }
        
        return EC_BIND;
    }

    return EC_OK;
}

errcode_t Server::Listen() {
    if (this->sock_fd < 0)
        TRACE_AND_RETURN(EC_SERVER_UNINITIALIZED)
    
    if (listen(this->sock_fd, TOTAL_ACTIVE_CLIENTS) < 0)
            TRACE_WITH_ERRNO_AND_RETURN(EC_LISTEN)

    return EC_OK;
}

errcode_t Server::AllocateSharedMemory() {
    queue = new MessageQueue();
    if (queue->Init() > 0)
        PANIC_WITH_CTX
    
    return EC_OK;
}

errcode_t Server::Run() {
    if (this->Bind() > 0)
        PANIC_WITH_CTX

    std::cout << "bind ok\n";

    if (this->Listen() > 0)
        PANIC_WITH_CTX

    std::cout << "listen ok\n";

    if (this->AllocateSharedMemory() > 0)
        PANIC_WITH_CTX

    std::cout << "alloc ok\n";

    MQCreds *shared_message_queue_credentials = queue->GetCreds();
    
    uint64_t number_of_active_clients = 0;

    int client_sock;
    struct sockaddr_un client_sockaddr;
    socklen_t client_socklen;

    while ((client_sock = accept(this->sock_fd, (struct sockaddr *) &client_sockaddr, &client_socklen)) > 0) {
        const uint16_t client_id = number_of_active_clients;
        number_of_active_clients++;

        if (queue->MakeClientAliveWithID(client_id) > 0)
            PANIC_WITH_CTX

        if (fork()) {
            // server process
            if (close(client_sock) < 0)
                TRACE_WITH_ERRNO_AND_RETURN(EC_CLOSE)

            continue;
        } else {
            // child (agent) process
            
            // NOTE: Should we do this? 
            //
            // delete queue;
            // queue = new MessageQueue();
            // if (queue->InitWithCreds(shared_message_queue_credentials) > 0)
            //     PANIC_WITH_CTX

            Agent agent(client_id, client_sock, queue);
            
            if (agent.Run() > 0)
                PANIC_WITH_CTX
        }
    }

    TRACE_WITH_ERRNO_AND_RETURN(EC_ACCEPT)
}