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

key_t mqClientsKey[MAX_CLIENTS];
key_t mqServerKey;

client_state clientsState[MAX_CLIENTS];

void createServerMq();
void handleMsg(struct ping_msg msg);
void connectNewClient(key_t mqKey);
void deleteClient(int clientID);
void listForClient(int clientID);
void connect2Clients(int cID1, int cID2);
void connectRequest(struct connect_request_msg msg);
void disconnectRequest(struct connect_request_msg msg);
void chatEnd(int clientID);
void stopServer();
void sigintHandler(){
    exit(0);
}

int main(int argc, char** argv){
    for(int i = 0; i < MAX_CLIENTS; ++i)
        clientsState[i] = NEXIST;
    createServerMq();
    atexit(&stopServer);
    signal(SIGINT, &sigintHandler);
    struct ping_msg msg;
    while(1){
        if(msgrcv(mqServerID, &msg, MAX_MSG, -MAX_CLIENT_MTYPE, 0) == -1){
            puts("Recieve error");
        }
        else{
            handleMsg(msg);
        }
    }
    return 0;
}

// PROGRAM BASIC MESSAGE QUEUE CREATING
void createServerMq(){
    char* homePath = getenv("HOME");
    do{
        mqServerKey = ftok(homePath, PROJ_ID);

        // TODO: errno handle
        mqServerID = msgget(mqServerKey, IPC_CREAT | S_IRWXU);
    }while(0);
}

///////////////////////////////////////
// CLIENT CONNECTING TO SERVER HANDLING

void connectNewClient(key_t mqKey){
    int clientID;
    for(int i = 0; i < MAX_CLIENTS; ++i){
        clientID = (connectedClients + i) % MAX_CLIENTS;
        if(clientsState[clientID] == NEXIST)
            break;
    }
    // MAX_CLIENTS achieved
    if(clientsState[clientID] != NEXIST){
        clientID = MAX_CLIENTS;
        mqClientsID[MAX_CLIENTS] = -1;
    }
    struct clientid_msg msg = {.mtype = ID_RESPONSE, .clientID = clientID};

    mqClientsID[clientID] = msgget(mqKey, 0);
    if(mqClientsID[clientID] == -1){
        puts("Sending message back to client failed");
    }
    else{
        msgsnd(mqClientsID[clientID], &msg, sizeof(int), 0);
        if(clientID != MAX_CLIENTS){
            printf("Connected %d\n", clientID);
            mqClientsKey[clientID] = mqKey;
            clientsState[clientID] = ACTIVE;
            connectedClients++;
        }
        else{
            puts("Client wanted to connect but MAX_CLIENTS reached");
        }
    }
}

//////////////////////////////////////////
// HANDLING MESSAGES RECEIVED FROM CLIENTS

void listForClient(int clientID){
    struct list_cell cell = {.mtype = LIST_CLIENT};
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clientsState[i] != NEXIST){
            cell.clientID = i;
            cell.clientState = clientsState[i];
            msgsnd(mqClientsID[clientID], &cell, sizeof(int) + sizeof(client_state), 0);
        }
    }
    cell.clientID = -1;
    msgsnd(mqClientsID[clientID], &cell, sizeof(int), 0);
}

void deleteClient(int clientID){
    mqClientsID[clientID] = -1;
    clientsState[clientID] = NEXIST;
    connectedClients--;
    printf("Deleted %d\n", clientID);
}

void connectRequest(struct connect_request_msg msg){
    if(msg.connectClientID != msg.requestClientID && clientsState[msg.connectClientID] == ACTIVE && clientsState[msg.requestClientID] == ACTIVE){
        connect2Clients(msg.requestClientID, msg.connectClientID);
    }
    else{
        printf("Couldn't connect %d and %d clients\n", msg.connectClientID, msg.requestClientID);
        struct connect_msg response = {.mtype = CONNECT_RESPONSE, .connectClientID = -1};
        msgsnd(mqClientsID[msg.requestClientID], &response, 2*sizeof(int), 0);
    }
}

void disconnectRequest(struct connect_request_msg msg){
    struct clientid_msg response = {.mtype = DISCONNECT_RESPONSE};
    if(clientsState[msg.connectClientID] == PRIVATE_CONNECT){
        msgsnd(mqClientsID[msg.connectClientID], &response, sizeof(int), 0);
        clientsState[msg.connectClientID] = ACTIVE;
    }
    if(clientsState[msg.requestClientID] == PRIVATE_CONNECT){
        msgsnd(mqClientsID[msg.requestClientID], &response, sizeof(int), 0);
        clientsState[msg.requestClientID] = ACTIVE;
    }
}

//////////////////////////////////////
// CREATING PRIVATE CHAT FOR 2 CLIENTS

void connect2Clients(int cID1, int cID2){
    struct connect_msg response1 = {.mtype = CONNECT_RESPONSE, .connectClientID = cID2, .connectClientKey = mqClientsKey[cID2]};
    msgsnd(mqClientsID[cID1], &response1, 2*sizeof(int), 0);

    struct connect_msg response2 = {.mtype = CONNECT_RESPONSE, .connectClientID = cID1, .connectClientKey = mqClientsKey[cID1]};
    msgsnd(mqClientsID[cID2], &response2, 2*sizeof(int), 0);

    clientsState[cID1] = clientsState[cID2] = PRIVATE_CONNECT;
}

///////////////////////////////////////////
// ACTIONS TO TAKE WHILE TERMINATING SERVER

void stopServer(){
    struct clientid_msg msg;
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clientsState[i] != NEXIST){
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

////////////////////////////////////////////////////
// CHOOSING ACTION IN TERMS OF MESSAGE TYPE (msg[0])

void handleMsg(struct ping_msg msg){
    switch(msg.mtype){ 
        case INIT:
            connectNewClient(((struct clientkey_msg*)&msg)->clientKey); break;
        case LIST:
            listForClient(((struct clientid_msg*)&msg)->clientID); break;
        case STOP:
            deleteClient(((struct clientid_msg*)&msg)->clientID); break;
        case CONNECT:
            connectRequest(*((struct connect_request_msg*)&msg)); break;
        case DISCONNECT:
            disconnectRequest(*((struct connect_request_msg*)&msg)); break;
        default:
            puts("Message undefined"); break;
    }
}