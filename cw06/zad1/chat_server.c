#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include "chat_flags.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int serverStopped = 0;

long connectedClients = 0;
int mqClientsID[MAX_CLIENTS];
int mqServerID;

key_t mqServerKey;

void createServerMq();
void handleMsg(struct ping_msg msg);
void connectNewClient(key_t mqID);
void listForClient(int clientID);

int main(int argc, char** argv){
    for(int i = 0; i < MAX_CLIENTS; ++i)
        mqClientsID[i] = -1;
    createServerMq();
    struct ping_msg msg;
    while(!serverStopped){
        if(msgrcv(mqServerID, &msg, MAX_MSG, 0, 0) == -1){
            switch (errno)
            {
            case EACCES:
                puts("1");
                break;
            case EAGAIN:
                puts("2");
                break;

            case EFAULT:
                puts("3");
                break;

            case EIDRM:
                puts("4");
                break;

            case EINTR:
                puts("5");
                break;

            case EINVAL:
                puts("6");
                break;

            case ENOMEM:
                puts("7");
                break;
            default:
                puts("None");
                break;
            }
        }
        else{
            handleMsg(msg);
        }
    }
    return 0;
}

void createServerMq(){
    char* homePath = getenv("HOME");
    if(homePath == NULL){
        puts("You are $HOMEless");
        homePath = "/home/pinio";
    }
    do{
        mqServerKey = ftok(homePath, PROJ_ID);

        // TODO: errno handle
        mqServerID = msgget(mqServerKey, IPC_CREAT | S_IRWXU);
    }while(0);
    printf("Chat server key: %d\n", mqServerKey);
}

void connectNewClient(key_t mqID){
    int clientID = connectedClients;
    struct clientid_msg msg = {.mtype = ID_RESPONSE, .clientID = clientID};

    mqClientsID[clientID] = msgget(mqID, 0);
    if(mqClientsID[clientID] == -1){
        puts("Sending message back to client failed");
    }
    else{
        msgsnd(mqClientsID[clientID], &msg, sizeof(int), 0);
        connectedClients++;
    }
}

void listForClient(int clientID){
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(mqClientsID[i] != -1){
            printf("ID: %d\tStatus: ACTIVE\n", i);
        }
    }
}

void handleMsg(struct ping_msg msg){
    switch(msg.mtype){ 
        case INIT:
            connectNewClient(((struct clientkey_msg*)&msg)->clientKey); break;
        case LIST:
            listForClient(((struct clientid_msg*)&msg)->clientID); break;
        default:
            puts("lolz"); break;
    }
}