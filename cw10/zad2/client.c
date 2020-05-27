#include "utils.h"

char* myName;
char* address;
char* mode;
int socketFd;
char mySign;
char oppSign;

void exitActions(){
    shutdown(socketFd, SHUT_RDWR);
    close(socketFd);
}

void exiter(){
    exit(-1);
}

int main(int argc, char** argv){
    if(argc < 4){
        puts("Not enough arguments");
        exit(-1);
    }
    atexit(exitActions);
    signal(SIGINT, exiter);

    myName = argv[1];
    mode = argv[2];
    address = argv[3];

    if(strcmp("UNIX", mode) == 0){
        struct sockaddr_un add = {.sun_family = AF_UNIX};
        strcpy(add.sun_path, address);

        socketFd = socket(AF_UNIX, SOCK_DGRAM, 0);
        connect(socketFd, (struct sockaddr*)&add, sizeof(add));
    }
    else if(strcmp("INET", mode) == 0){
        struct sockaddr_in add = {.sin_family = AF_INET};
    
        add.sin_family = AF_INET; 
        char* part = strtok(address, ":");
        inet_pton(AF_INET, part, &add.sin_addr);
        
        part = strtok(NULL, ":");
        add.sin_port = htons(atoi(part)); 
        
        socketFd = socket(AF_INET, SOCK_DGRAM, 0);
        connect(socketFd, (struct sockaddr*)&add, sizeof(add));
    }
    
    char* msg = (char*)malloc(MAX_MSG*sizeof(char));
    strcpy(&msg[1], myName);
    msg[0] = msg_name;

    send(socketFd, msg, MAX_MSG, 0);
    recv(socketFd, msg, MAX_MSG, MSG_WAITALL);

    if(msg[0] != msg_logged)
        exit(-1);
    else
        printInfo("Logged");


    while(msg[0] != msg_start){
        recv(socketFd, msg, MAX_MSG, MSG_WAITALL);
        if(msg[0] == msg_ping){
            send(socketFd, msg, MAX_MSG, 0);
        }
    }

    Board* board = initBoard();
    char winner = emptySign;
    mySign = msg[1];
    oppSign = (mySign == xSign) ? oSign : xSign;
    char sign = xSign;

    struct epoll_event events[5];
    
    struct epoll_event ev;
    ev.data.fd = socketFd;
    ev.events = EPOLLIN;

    int epollFd = epoll_create1(0);
    epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, &ev);
    while(winner == emptySign){
        system("clear");
        printBoard(board);
        int i;
        if(sign == mySign) {
            printf("Place %c\n", mySign);
            ev.data.fd = STDIN_FILENO;
            ev.events = EPOLLIN;
            epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
            epoll_wait(epollFd, events, 5, -1);
            if(events[0].data.fd == STDIN_FILENO){
                scanf("%d", &i);
                if(placeSign(board, mySign, (i-1)/3, (i-1)%3) == 0){
                    msg[0] = msg_move;
                    msg[1] = (char)i;
                    send(socketFd, msg, MAX_MSG, 0);
                    winner = checkWin(board);
                    if(sign == xSign)
                        sign = oSign;
                    else
                        sign = xSign;
                }
                fflush(stdin);
                epoll_ctl(epollFd, EPOLL_CTL_DEL, STDIN_FILENO, &ev);
            }
            else{
                recv(socketFd, msg, MAX_MSG, MSG_WAITALL);
                send(socketFd, msg, MAX_MSG, 0);
            }
        }
        else{
            printf("Waiting for opponent's move\n");
            recv(socketFd, msg, MAX_MSG, MSG_WAITALL);
            if(msg[0] == msg_move){
                i = msg[1];
                placeSign(board, oppSign, (i-1)/3, (i-1)%3);
                winner = checkWin(board);
                if(sign == xSign)
                    sign = oSign;
                else
                    sign = xSign;
            }
            else if(msg[0] == msg_ping){
                send(socketFd, msg, MAX_MSG, 0);
                printInfo("Ponged");
            }
        }
    }
    system("clear");
    printBoard(board);
    if(winner == -1){
        puts("Winning is not the most important");
        puts("Having fun is");
    }
    else if(winner == emptySign){
        printError("Server problem or opponent disconnected");
    }
    else
        printf("Winner is %c\n", winner);
    
    msg[0] = msg_disconnected;
    send(socketFd, msg, MAX_MSG, 0);
    return 0;
}