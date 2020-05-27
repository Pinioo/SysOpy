#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <sys/signal.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_CLIENTS 16
#define MAX_NAME 32
#define MAX_MSG 33


const int EMPTY = -1;
const int INET = 0;
const int UNIX = 1;

const char emptySign = ' ';
const char xSign = 'X';
const char oSign = 'O';

const char msg_name = 0;
const char msg_logged = 1;
const char msg_disconnected = 2;
const char msg_move = 3;
const char msg_start = 4;
const char msg_ping = 5;

typedef struct Board{
    char board[3][3];
} Board;

Board* initBoard(){
    Board* b = (Board*)malloc(sizeof(Board));
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; j++)
            b->board[i][j] = emptySign;
    return b;
}

int placeSign(Board* b, char sign, int i, int j){
    if((sign == xSign || sign == oSign) && i >= 0 && i < 3 && j >= 0 && j < 3 && b->board[i][j] == emptySign){
        b->board[i][j] = sign;
        return 0;
    }
    else
        return -1;
}

int checkWin(Board* b){
    int sameSign;
    char lastSign;
    for(int i = 0; i < 3; ++i){
        sameSign = 1;
        lastSign = emptySign;
        for(int j = 0; j < 3; j++){
            if(b->board[i][j] != emptySign && (lastSign == emptySign || b->board[i][j] == lastSign))
                lastSign = b->board[i][j];
            else{
                sameSign = 0;
                break;
            }
        }
        if(sameSign)
            return lastSign;

        
        sameSign = 1;
        lastSign = emptySign;
        for(int j = 0; j < 3; j++){
            if(b->board[j][i] != emptySign && (lastSign == emptySign || b->board[j][i] == lastSign))
                lastSign = b->board[j][i];
            else{
                sameSign = 0;
                break;
            }
        }
        if(sameSign)
            return lastSign;
    }

    sameSign = 1;
    lastSign = emptySign;
    for(int i = 0; i < 3; ++i){
        if(b->board[i][i] != emptySign && (lastSign == emptySign || b->board[i][i] == lastSign))
            lastSign = b->board[i][i];
        else{
            sameSign = 0;
            break;
        }
    }
    if(sameSign)
        return lastSign;

    sameSign = 1;
    lastSign = emptySign;
    for(int i = 0; i < 3; ++i){
        if(b->board[i][2-i] != emptySign && (lastSign == emptySign || b->board[i][2-i] == lastSign))
            lastSign = b->board[i][2-i];
        else{
            sameSign = 0;
            break;
        }
    }
    if(sameSign)
        return lastSign;

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            if(b->board[i][j] == emptySign)
                return emptySign;
    
    return -1;
}

void printBoard(Board* b){
    puts("-------------------");
    for(int row = 0; row < 3; row++){
        printf("|[%d] %c|[%d] %c|[%d] %c|\n", row*3+1, b->board[row][0], row*3+2, b->board[row][1], row*3+3, b->board[row][2]);
    }
    puts("-------------------");
}

void printInfo(char* info){
    printf("[INFO] %s\n", info);
}

void printError(char* info){
    printf("[ERROR] %s\n", info);
}
