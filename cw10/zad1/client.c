#include "utils.h"

char* myName;
char* address;
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
    address = argv[3];

    struct sockaddr_un add = {.sun_family = AF_UNIX};
    strcpy(add.sun_path, address);

    socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(socketFd, &add, sizeof(add));

    char* msg = (char*)malloc(MAX_MSG*sizeof(char));
    strcpy(&msg[1], myName);
    msg[0] = msg_name;

    send(socketFd, msg, MAX_MSG, 0);
    recv(socketFd, msg, MAX_MSG, MSG_WAITALL);

    if(msg[0] != msg_logged)
        exit(-1);
    else
        printInfo("Logged");

    recv(socketFd, msg, MAX_MSG, MSG_WAITALL);

    if(msg[0] != msg_start)
        exit(-1);

    Board* board = initBoard();
    char winner = emptySign;
    mySign = msg[1];
    oppSign = (mySign == xSign) ? oSign : xSign;
    char sign = xSign;
    while(winner == emptySign){
        system("clear");
        printBoard(board);
        int i;
        if(sign == mySign) {
            printf("Place %c: ", mySign);
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
        }
        else{
            printf("Waiting for opponent's move\n", oppSign);
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