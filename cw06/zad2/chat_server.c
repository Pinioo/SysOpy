#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include "chat_flags.h"
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <fcntl.h>

int serverStopped = 0;

long connectedClients = 0;
mqd_t mqClientsID[MAX_CLIENTS + 1];
mqd_t mqServerID;

char mqClientsName[MAX_CLIENTS][MQ_NAME_LEN + 1];

client_state clientsState[MAX_CLIENTS];

void createServerMq();
void handleMsg(char* msg);
void connectNewClient(char* mqName);
void deleteClient(int clientID);
void listForClient(int clientID);
void connect2Clients(int cID1, int cID2);
void connectRequest(char*);
void disconnectRequest(char*);
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
    char msg[MAX_MSG];
    while(1){
        if(mq_receive(mqServerID, msg, MAX_MSG, NULL) == -1){
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
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG;
    attr.mq_flags = 0;
    if((mqServerID = mq_open(SERVER_MQ_NAME, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &attr)) == -1){
        puts("Error during server queue creation");
        exit(-1);
    }
}

///////////////////////////////////////
// CLIENT CONNECTING TO SERVER HANDLING

void connectNewClient(char* mqName){
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
    char msg[MAX_MSG] = {ID_RESPONSE, (char)clientID};

    mqClientsID[clientID] = mq_open(mqName, O_WRONLY);
    if(mqClientsID[clientID] == -1){
        puts("Sending message back to client failed");
    }
    else{
        mq_send(mqClientsID[clientID], msg, MAX_MSG, 8);
        if(clientID != MAX_CLIENTS){
            printf("Connected %d\n", clientID);
            strcpy(mqClientsName[clientID], mqName);
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

void deleteClient(int clientID){
    mq_close(mqClientsID[clientID]);
    mqClientsID[clientID] = -1;
    clientsState[clientID] = NEXIST;
    connectedClients--;
    printf("Deleted %d\n", clientID);
}

void listForClient(int clientID){
    char cell[MAX_MSG] = {LIST_CLIENT, 0, 0}; 
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clientsState[i] != NEXIST){
            cell[1] = i;
            cell[2] = clientsState[i];
            mq_send(mqClientsID[clientID], cell, MAX_MSG, 5);
        }
    }
    cell[1] = -1;
    mq_send(mqClientsID[clientID], cell, MAX_MSG, 5);
}

void connectRequest(char* msg){
    int connectClientID = (int)msg[0];
    int requestClientID = (int)msg[1];
    if(connectClientID != requestClientID && clientsState[connectClientID] == ACTIVE && clientsState[requestClientID] == ACTIVE){
        connect2Clients(requestClientID, connectClientID);
    }
    else{
        printf("Couldn't connect %d and %d clients\n", connectClientID, requestClientID);
        char response[MAX_MSG] = {CONNECT_RESPONSE, -1};
        mq_send(mqClientsID[requestClientID], response, MAX_MSG, 0);
    }
}

void disconnectRequest(char* msg){
    char response[MAX_MSG];
    response[0] = DISCONNECT_RESPONSE;
    int cID1 = (int)msg[0];
    int cID2 = (int)msg[1];
    if(clientsState[cID1] == PRIVATE_CONNECT){
        mq_send(mqClientsID[cID1], response, MAX_MSG, 0);
        clientsState[cID1] = ACTIVE;
    }
    if(clientsState[cID2] == PRIVATE_CONNECT){
        mq_send(mqClientsID[cID2], response, MAX_MSG, 0);
        clientsState[cID2] = ACTIVE;
    }
}

//////////////////////////////////////
// CREATING PRIVATE CHAT FOR 2 CLIENTS

void connect2Clients(int cID1, int cID2){
    char response1[MAX_MSG];
    response1[0] = CONNECT_RESPONSE;
    response1[1] = (char)cID2;
    for(size_t i = 2; i < 3 + MQ_NAME_LEN; ++i)
        response1[i] = mqClientsName[cID2][i-2];
    mq_send(mqClientsID[cID1], response1, MAX_MSG, 0);

    char response2[MAX_MSG];
    response2[0] = CONNECT_RESPONSE;
    response2[1] = (char)cID1;
    for(size_t i = 2; i < 3 + MQ_NAME_LEN; ++i)
        response2[i] = mqClientsName[cID1][i-2];
    mq_send(mqClientsID[cID2], response2, MAX_MSG, 0);

    clientsState[cID1] = clientsState[cID2] = PRIVATE_CONNECT;
}

///////////////////////////////////////////
// ACTIONS TO TAKE WHILE TERMINATING SERVER

void stopServer(){
    char stopMsg[MAX_MSG];
    char responseMsg[MAX_MSG];
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clientsState[i] != NEXIST){
            stopMsg[0] = SERVER_STOP;
            if(mq_send(mqClientsID[i], stopMsg, MAX_MSG, 10) == -1){
                printf("Couldn't ping client with ID: %d\n", i);
            }
            else do{
                mq_receive(mqServerID, responseMsg, MAX_MSG, NULL);
                if(responseMsg[0] == STOP)
                    deleteClient(responseMsg[1]);
            }while(responseMsg[0] != STOP || responseMsg[1] != i);
        }
    }
    mq_close(mqServerID);
    mq_unlink(SERVER_MQ_NAME);
}

////////////////////////////////////////////////////
// CHOOSING ACTION IN TERMS OF MESSAGE TYPE (msg[0])

void handleMsg(char* msg){
    switch(msg[0]){ 
        case INIT:
            connectNewClient(&msg[1]); break;
        case LIST:
            listForClient((int)msg[1]); break;
        case STOP:
            deleteClient((int)msg[1]); break;
        case CONNECT:
            connectRequest(&msg[1]); break;
        case DISCONNECT:
            disconnectRequest(&msg[1]); break;
        default:
            puts("Message undefined"); break;
    }
}