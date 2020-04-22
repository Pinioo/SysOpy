#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <string.h>
#include "chat_flags.h"

pid_t child;

key_t mqPrivateKey;
key_t mqServerKey;

int mqPrivateID;
int mqServerID;

int clientID;

void stop();
void list();
void openQueue();
int connectToServer();
void sendMessage(char*);
void sigintHandler(){
    exit(0);
}

int main(int argc, char** argv){
    srand(time(NULL));
    openQueue();
    if(connectToServer() == -1){
        puts("Couldn't connect to server");
        msgctl(mqPrivateID, IPC_RMID, NULL);
        exit(-1);
    }
    printf("ID: %d\n", clientID);
    child = fork();
    if(child == 0){
        pid_t parent = getppid();
        struct ping_msg stopped;
        msgrcv(mqPrivateID, &stopped, MAX_MSG, SERVER_STOP, 0);
        kill(parent, SIGINT);
        pause();
        return 0;
    }
    else{
        atexit(&stop);
        signal(SIGINT, &sigintHandler);
        char* command = (char*)malloc(128*sizeof(char));
        while(1){
            scanf("%s", command);
            if(strcmp(command, "list") == 0)
                list();
            else if(strcmp(command, "send") == 0)
                sendMessage(NULL);
            else if(strcmp(command, "stop") == 0)
                exit(0);
            else
                puts("Invalid command");
        }
        return 0;
    }
}

void openQueue(){
    char* homePath = getenv("HOME");
    do{
        mqPrivateKey = ftok(homePath, rand());
        mqPrivateID = msgget(mqPrivateKey, IPC_CREAT | IPC_EXCL | S_IRWXU);
    }while(mqPrivateID == -1);
}

void stop(){
    struct clientid_msg stopMsg = {.mtype = STOP, .clientID = clientID};
    msgsnd(mqServerID, &stopMsg, sizeof(int), 0);
    msgctl(mqPrivateID, IPC_RMID, NULL);
    kill(child, SIGKILL);
    exit(0);
}

int connectToServer(){
    char* homePath = getenv("HOME");
    mqServerKey = ftok(homePath, PROJ_ID);
    mqServerID = msgget(mqServerKey, 0);

    struct clientkey_msg keyMsg = {.mtype = INIT, .clientKey = mqPrivateKey};
    msgsnd(mqServerID, &keyMsg, MAX_MSG, 0);

    struct ping_msg response;
    msgrcv(mqPrivateID, &response, MAX_MSG, ID_RESPONSE, 0);
    clientID = ((struct clientid_msg*)&response)->clientID;
    if(clientID != MAX_CLIENTS)
        return 0;
    else
        return -1;
}

void list(){
    struct clientid_msg idMsg = {.mtype = LIST, .clientID = clientID};
    msgsnd(mqServerID, &idMsg, sizeof(int), 0);

    struct list_cell cellRecieved;
    msgrcv(mqPrivateID, &cellRecieved, sizeof(int), 0, 0);
    while(cellRecieved.mtype == LIST_CLIENT){
        printf("ClientID: %d\n", cellRecieved.clientID);
        msgrcv(mqPrivateID, &cellRecieved, sizeof(int), 0, 0);
    }
}

void sendMessage(char* msg){
    puts("PLACEHOLDER");
}