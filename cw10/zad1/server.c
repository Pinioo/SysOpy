#include "utils.h"

pthread_mutex_t mute = PTHREAD_MUTEX_INITIALIZER;

int unixSocketCreated = 0;
int inetSocketCreated = 0;

int epollFd;
int waitingClient = -1;
int clientSocketFd[MAX_CLIENTS];
int opponents[MAX_CLIENTS];
int ponged[MAX_CLIENTS];
char clientName[MAX_CLIENTS][MAX_NAME];

u_int16_t tcpSocketPort;
char info[128];
char* unixSocketName;
struct epoll_event events[5];

int unixSocketFd;
int inetSocketFd;

void* pinger(){
    while(1){
        pthread_mutex_lock(&mute);
        printInfo("Pinging");
        for(int i = 0; i < MAX_CLIENTS; i++){
            char msg[MAX_MSG];
            msg[0] = msg_ping;
            if(clientSocketFd[i] != -1){
                if(!ponged[i] || send(clientSocketFd[i], msg, MAX_MSG, 0) == -1){
                    close(clientSocketFd[i]);
                    if(opponents[i]){
                        msg[0] = msg_disconnected;
                        send(clientSocketFd[opponents[i]], msg, MAX_MSG, 0);
                        close(clientSocketFd[opponents[i]]);
                        clientName[opponents[i]][0] = '\0';
                        clientSocketFd[opponents[i]] = -1;
                        opponents[opponents[i]] = -1;
                    }
                    clientName[i][0] = '\0';
                    opponents[i] = -1;
                    clientSocketFd[i] = -1;
                }
                ponged[i] = 0;
            }
        }
        pthread_mutex_unlock(&mute);
        sleep(6);
    }
    return NULL;
}

int findFdIndex(int fd){
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(clientSocketFd[i] == fd)
            return i;
    return -1;
}

int findNextFdPlace(){
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(clientSocketFd[i] == -1)
            return i;
    return -1;
}

int isNameUnique(char* name){
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(strcmp(name, clientName[i]) == 0)
            return 0;
    return 1;
}

void exitActions(){
    if(unixSocketCreated){
        unlink(unixSocketName);
    }
}

void placeHolder(){
    exit(-1);
}

int main(int argc, char** argv){
    if(argc < 3){
        puts("Not enough arguments");
        exit(-1);
    }
    atexit(exitActions);
    signal(SIGINT, placeHolder);

    printInfo("Server starting");

    for(int i = 0; i < MAX_CLIENTS; i++){
        clientSocketFd[i] = -1;
        opponents[i] = -1;
        clientName[i][0] = '\0';
        ponged[i] = 1;
    }

    epollFd = epoll_create1(0);
    struct epoll_event ev = {.events = EPOLLIN};

    tcpSocketPort = atoi(argv[1]);
    unixSocketName = argv[2];

    unixSocketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    inetSocketFd = socket(AF_INET, SOCK_STREAM, 0);

    if(unixSocketFd == -1 || inetSocketFd == -1){
        printError("Socket cannot be created");
        exit(-1);
    }
    struct sockaddr_un unixAddr = {.sun_family = AF_UNIX};
    strcpy(unixAddr.sun_path, unixSocketName);

    if(bind(unixSocketFd, (struct sockaddr*)&unixAddr, sizeof(struct sockaddr_un)) == -1){
        printError("UNIX socket address is unavailable");
        exit(-1);
    }
    else
        unixSocketCreated = 1;

    struct sockaddr_in inetAddr = {.sin_family = AF_INET, .sin_port = htobe16(tcpSocketPort)};
    inetAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(inetSocketFd, (struct sockaddr*)&inetAddr, sizeof(struct sockaddr_in)) == -1){
        printError("TCP socket address is unavailable");
        exit(-1);
    }
    else
        inetSocketCreated = 1;

    sprintf(info, "UNIX socket available at %s", unixSocketName);
    printInfo(info);

    sprintf(info, "Server running at 127.0.0.1:%d", tcpSocketPort);
    printInfo(info);

    char* msg = (char*)malloc(MAX_MSG*sizeof(char));

    ev.data.fd = unixSocketFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, unixSocketFd, &ev);

    ev.data.fd = inetSocketFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, inetSocketFd, &ev);
    
    listen(unixSocketFd, 5);
    listen(inetSocketFd, 5);

    pthread_t threadId;
    pthread_create(&threadId, NULL, pinger, NULL);

    while(1){
        int event_count = epoll_wait(epollFd, events, 5, -1);
        pthread_mutex_lock(&mute);
        for(int i = 0; i < event_count; i++){
            if(events[i].data.fd == unixSocketFd || events[i].data.fd == inetSocketFd){
                int id = findNextFdPlace();
                clientSocketFd[id] = accept(events[i].data.fd, NULL, NULL);

                if(clientSocketFd[id] == -1){
                    printError("Accept problem");
                    break;
                }
                else {
                    recv(clientSocketFd[id], msg, MAX_MSG, MSG_WAITALL);
                    if(msg[0] == msg_name){
                        if(isNameUnique(&msg[1])){
                            strcpy(clientName[id], &msg[1]);
                            msg[0] = msg_logged;
                            send(clientSocketFd[id], msg, MAX_MSG, 0);
                            if(waitingClient == -1)
                                waitingClient = id;
                            else{
                                msg[0] = msg_start;
                                opponents[id] = waitingClient;
                                opponents[waitingClient] = id;

                                waitingClient = -1;
                                msg[1] = oSign;
                                send(clientSocketFd[id], msg, MAX_MSG, 0);

                                msg[1] = xSign;
                                send(clientSocketFd[opponents[id]], msg, MAX_MSG, 0);
                            }

                            ev.data.fd = clientSocketFd[id];
                            epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocketFd[id], &ev);
                            printInfo("New client connected");
                        }
                        else{
                            msg[0] = msg_disconnected;
                            send(clientSocketFd[id], msg, MAX_MSG, 0);
                            close(clientSocketFd[id]);
                            clientSocketFd[id] = -1;
                        }
                    }
                }
            }
            else {
                int id = findFdIndex(events[i].data.fd);
                recv(clientSocketFd[id], msg, MAX_MSG, MSG_WAITALL);
                if(msg[0] == msg_disconnected){
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocketFd[id], &ev);
                    close(events[i].data.fd);
                    opponents[id] = -1;
                    clientName[id][0] = -1;
                    clientSocketFd[id] = -1;
                    ponged[i] = 1;
                    printInfo("Client disconnected");
                }

                else if(msg[0] == msg_move){
                    send(clientSocketFd[opponents[id]], msg, MAX_MSG, 0);
                }
                
                else if(msg[0] == msg_ping){
                    ponged[id] = 1;
                }
            }
        }
        pthread_mutex_unlock(&mute);
    }
    return 0;
}