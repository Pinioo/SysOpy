#include "utils.h"

int unixSocketCreated = 0;
int inetSocketCreated = 0;

int epollFd;
int waitingClient = -1;
int slotStatus[MAX_CLIENTS];
struct sockaddr_in clientInet[MAX_CLIENTS];
struct sockaddr_un clientUnix[MAX_CLIENTS];
int opponents[MAX_CLIENTS];
int ponged[MAX_CLIENTS];
char clientName[MAX_CLIENTS][MAX_NAME];

pthread_mutex_t mute = PTHREAD_MUTEX_INITIALIZER;

u_int16_t udpSocketPort;
char info[128];
char* unixSocketName;
struct epoll_event events[5];

int unixSocketFd;
int inetSocketFd;

int sendMsg(int id, char* msg, size_t msgSize){
    if(slotStatus[id] == UNIX)
        return sendto(unixSocketFd, msg, MAX_MSG, 0, (struct sockaddr*)&clientUnix[id], sizeof(struct sockaddr_un));
    if(slotStatus[id] == INET)
        return sendto(inetSocketFd, msg, MAX_MSG, 0, (struct sockaddr*)&clientInet[id], sizeof(struct sockaddr_in));
    return -1;
}

void* pinger(){
    while(1){
        pthread_mutex_lock(&mute);
        printInfo("Pinging");
        for(int i = 0; i < MAX_CLIENTS; i++){
            char msg[MAX_MSG];
            msg[0] = msg_ping;
            if(slotStatus[i] != EMPTY){
                if(!ponged[i] || sendMsg(i, msg, MAX_MSG) == -1){
                    if(opponents[i]){
                        msg[0] = msg_disconnected;
                        sendMsg(opponents[i], msg, MAX_MSG);
                        clientName[opponents[i]][0] = '\0';
                        slotStatus[opponents[i]] = EMPTY;
                        opponents[opponents[i]] = -1;
                    }
                    clientName[i][0] = '\0';
                    opponents[i] = -1;
                    slotStatus[i] = EMPTY;
                }
                ponged[i] = 0;
            }
        }
        pthread_mutex_unlock(&mute);
        sleep(6);
    }
    return NULL;
}

int findInIndex(struct sockaddr_in add){
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(slotStatus[i] == INET && add.sin_port == clientInet[i].sin_port)
            return i;
    }
    return -1;
}

int findUnIndex(struct sockaddr_un add){
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(slotStatus[i] == UNIX && strcmp(add.sun_path, clientUnix[i].sun_path) == 0)
            return i;
    }
    return -1;
}

int findNextFdPlace(){
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(slotStatus[i] == EMPTY)
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
        shutdown(unixSocketFd, SHUT_RDWR);
        shutdown(inetSocketFd, SHUT_RDWR);
        close(unixSocketFd);
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
        slotStatus[i] = EMPTY;
        opponents[i] = -1;
        clientName[i][0] = '\0';
        ponged[i] = 1;
    }

    epollFd = epoll_create1(0);
    struct epoll_event ev = {.events = EPOLLIN};

    udpSocketPort = atoi(argv[1]);
    unixSocketName = argv[2];

    unixSocketFd = socket(AF_UNIX, SOCK_DGRAM, 0);
    inetSocketFd = socket(AF_INET, SOCK_DGRAM, 0);

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

    struct sockaddr_in inetAddr = {.sin_family = AF_INET, .sin_port = htobe16(udpSocketPort)};
    inetAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(inetSocketFd, (struct sockaddr*)&inetAddr, sizeof(struct sockaddr_in)) == -1){
        printError("UDP socket address is unavailable");
        exit(-1);
    }
    else
        inetSocketCreated = 1;

    sprintf(info, "UNIX socket available at %s", unixSocketName);
    printInfo(info);

    sprintf(info, "Server running at 127.0.0.1:%d", udpSocketPort);
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
            if(events[i].data.fd == unixSocketFd){
                struct sockaddr_un unAddr;
                socklen_t sockLen;
                recvfrom(unixSocketFd, msg, MAX_MSG, MSG_WAITALL, (struct sockaddr*)&unAddr, &sockLen);
                printf("%s\n", unAddr.sun_path);
                if(msg[0] == msg_name){
                    int id = findNextFdPlace();
                    clientUnix[id].sun_family = unAddr.sun_family;
                    strcpy(clientUnix[id].sun_path, unAddr.sun_path);
                    slotStatus[id] = UNIX;
                    if(isNameUnique(&msg[1])){
                        strcpy(clientName[id], &msg[1]);
                        msg[0] = msg_logged;
                        sendMsg(id, msg, MAX_MSG);
                        if(waitingClient == -1)
                            waitingClient = id;
                        else{
                            msg[0] = msg_start;
                            opponents[id] = waitingClient;
                            opponents[waitingClient] = id;

                            waitingClient = -1;
                            msg[1] = oSign;
                            sendMsg(id, msg, MAX_MSG);

                            msg[1] = xSign;
                            sendMsg(opponents[id], msg, MAX_MSG);
                        }
                        printInfo("New client connected");
                    }
                    else{
                        msg[0] = msg_disconnected;
                        sendMsg(id, msg, MAX_MSG);
                        slotStatus[id] = EMPTY;
                    }
                }
                else {
                    int id = findUnIndex(unAddr);
                    if(msg[0] == msg_disconnected){
                        opponents[id] = -1;
                        clientName[id][0] = -1;
                        slotStatus[id] = EMPTY;
                        ponged[i] = 1;
                        printInfo("Client disconnected");
                    }

                    else if(msg[0] == msg_move){
                        sendMsg(opponents[id], msg, MAX_MSG);
                    }
                    
                    else if(msg[0] == msg_ping){
                        ponged[id] = 1;
                    }
                }
            }
            else {
                struct sockaddr_in inAddr;
                socklen_t sockLen;
                recvfrom(inetSocketFd, msg, MAX_MSG, MSG_WAITALL, (struct sockaddr*)&inAddr, &sockLen);
                printf("%d\n", inAddr.sin_port);
                if(msg[0] == msg_name){
                    int id = findNextFdPlace();
                    clientInet[id].sin_family = inAddr.sin_family;
                    clientInet[id].sin_addr.s_addr = inAddr.sin_addr.s_addr;
                    clientInet[id].sin_port = inAddr.sin_port;
                    strcpy((char*)clientInet[id].sin_zero, (char*)inAddr.sin_zero);
                    slotStatus[id] = INET;
                    if(isNameUnique(&msg[1])){
                        strcpy(clientName[id], &msg[1]);
                        msg[0] = msg_logged;
                        sendMsg(id, msg, MAX_MSG);
                        if(waitingClient == -1)
                            waitingClient = id;
                        else{
                            msg[0] = msg_start;
                            opponents[id] = waitingClient;
                            opponents[waitingClient] = id;

                            waitingClient = -1;
                            msg[1] = oSign;
                            sendMsg(id, msg, MAX_MSG);

                            msg[1] = xSign;
                            sendMsg(opponents[id], msg, MAX_MSG);
                        }
                        printInfo("New client connected");
                    }
                    else{
                        msg[0] = msg_disconnected;
                        sendMsg(id, msg, MAX_MSG);
                        slotStatus[id] = EMPTY;
                    }
                }
                else {
                    int id = findInIndex(inAddr);
                    if(msg[0] == msg_disconnected){
                        opponents[id] = -1;
                        clientName[id][0] = -1;
                        slotStatus[id] = EMPTY;
                        ponged[i] = 1;
                        printInfo("Client disconnected");
                    }

                    else if(msg[0] == msg_move){
                        sendMsg(opponents[id], msg, MAX_MSG);
                    }
                    
                    else if(msg[0] == msg_ping){
                        ponged[id] = 1;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mute);
    }
    return 0;
}