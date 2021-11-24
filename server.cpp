#include "server.h"
#include "agent.h"

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
    
    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* by using AF_UNIX for the family and */
    /* giving it a filepath to bind to.    */
    /*                                     */
    /* Unlink the file so the bind will    */
    /* succeed, then bind to that file.    */
    /***************************************/

    server_sockaddr.sun_family = AF_UNIX;   
    strncpy(server_sockaddr.sun_path, this->sock_path.c_str(), sizeof(server_sockaddr.sun_path));

    if (unlink(this->sock_path.c_str()) < 0)
        TRACE_WITH_ERRNO_AND_RETURN(EC_UNLINK)

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
    // __error_trace_glob
  
    // // shmget returns an identifier in shmid
    // int shmid = shmget(key,1024,0666|IPC_CREAT);
  
    // // shmat to attach to shared memory
    // char *str = (char*) shmat(shmid,(void*)0,0);
  
    // cout<<"Write Data : ";
    // gets(str);
  
    // printf("Data written in memory: %s\n",str);
      
    // //detach from shared memory 
    // shmdt(str);
    return EC_OK;
}

errcode_t Server::PreserveSynchronization() {
    return EC_OK;
    // TODO
}

errcode_t Server::Run() {
    if (this->Bind() < 0)
        PANIC_WITH_CTX

    if (this->Listen() < 0)
        PANIC_WITH_CTX

    int client_sock;
    struct sockaddr_un client_sockaddr;
    socklen_t client_socklen;

    while ((client_sock = accept(this->sock_fd, (struct sockaddr *) &client_sockaddr, &client_socklen)) > 0) {
        if (fork()) {
            // server process
            if (close(client_sock) < 0)
                TRACE_WITH_ERRNO_AND_RETURN(EC_CLOSE)

            continue;
        } else {
            // child (agent) process
            Agent agent(client_sock);
            
            if (agent.Run() < 0)
                PANIC_WITH_CTX
        }
    }

    TRACE_WITH_ERRNO_AND_RETURN(EC_ACCEPT)
}