#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include "chat_flags.h"
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <fcntl.h>

int serverStopped = 0;

long connectedClients = 0;
int mqClientsID[MAX_CLIENTS + 1];
int mqServerID;

key_t mqServerKey;

void createServerMq();
void handleMsg(struct ping_msg msg);
void connectNewClient(key_t mqID);
void deleteClient(int clientID);
void listForClient(int clientID);
void stopServer();
void sigintHandler(){
    exit(0);
}

int main(int argc, char** argv){
    for(int i = 0; i < MAX_CLIENTS; ++i)
        mqClientsID[i] = -1;
    createServerMq();
    atexit(&stopServer);
    signal(SIGINT, &sigintHandler);
    struct ping_msg msg;
    while(1){
        if(msgrcv(mqServerID, &msg, MAX_MSG, 0, 0) == -1){
            puts("Recieve error");
        }
        else{
            handleMsg(msg);
        }
    }
    return 0;
}

void createServerMq(){
    char* homePath = getenv("HOME");
    do{
        mqServerKey = ftok(homePath, PROJ_ID);

        // TODO: errno handle
        mqServerID = msgget(mqServerKey, IPC_CREAT | S_IRWXU);
    }while(0);
}

void connectNewClient(key_t mqID){
    int clientID;
    for(int i = 0; i < MAX_CLIENTS; ++i){
        clientID = (connectedClients + i) % MAX_CLIENTS;
        if(mqClientsID[clientID] == -1)
            break;
    }
    // MAX_CLIENTS achieved
    if(mqClientsID[clientID] != -1){
        clientID = MAX_CLIENTS;
        mqClientsID[MAX_CLIENTS] = -1;
    }
    struct clientid_msg msg = {.mtype = ID_RESPONSE, .clientID = clientID};

    mqClientsID[clientID] = msgget(mqID, 0);
    if(mqClientsID[clientID] == -1){
        puts("Sending message back to client failed");
    }
    else{
        msgsnd(mqClientsID[clientID], &msg, sizeof(int), 0);
        if(clientID != MAX_CLIENTS){
            printf("Connecting %d\n", clientID);
            connectedClients++;
        }
        else{
            puts("Client wanted to connect but MAX_CLIENTS reached");
        }
    }
}

void listForClient(int clientID){
    struct list_cell cell = {.mtype = LIST_CLIENT};
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(mqClientsID[i] != -1){
            cell.clientID = i;
            msgsnd(mqClientsID[clientID], &cell, sizeof(int), 0);
        }
    }
    cell.mtype = LIST_END;
    msgsnd(mqClientsID[clientID], &cell, sizeof(int), 0);
}

void deleteClient(int clientID){
    mqClientsID[clientID] = -1;
    connectedClients--;
    printf("%ld\n", connectedClients);
}

void handleMsg(struct ping_msg msg){
    switch(msg.mtype){ 
        case INIT:
            connectNewClient(((struct clientkey_msg*)&msg)->clientKey); break;
        case LIST:
            listForClient(((struct clientid_msg*)&msg)->clientID); break;
        case STOP:
            deleteClient(((struct clientid_msg*)&msg)->clientID); break;
        default:
            puts("lolz"); break;
    }
}

void stopServer(){
    struct clientid_msg msg = {.mtype = SERVER_STOP};
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(mqClientsID[i] != -1){
            msg.mtype = SERVER_STOP;
            if(msgsnd(mqClientsID[i], &msg, sizeof(int), 0) == -1){
                printf("Couldn't ping client with ID: %d\n", i);
            }
            else do{
                msgrcv(mqServerID, &msg, sizeof(int), STOP, 0);
                deleteClient(msg.clientID);
            }while(msg.clientID != i);
        }
    }
    msgctl(mqServerID, IPC_RMID, NULL);
}