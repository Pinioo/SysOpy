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

////////////////////////////////////////////////////////////////////////////////
// COMMANDS: LIST | STOP | CHECK | DISCONNECT | CONNECT (ID) | SEND (MESSAGE) //
////////////////////////////////////////////////////////////////////////////////

key_t mqPrivateKey;
key_t mqServerKey;

int mqPrivateID;
int mqServerID;

int mqPrivateChatID;

int clientID;
int privateChatID = -1;

void stop();
void list();
void openQueue();
int connectToServer();
void sendMessage(char*);
void requestConnect(int connectClientID);
void requestDisconnect();
void connectResponseHandler();
void disconnectResponseHandler();
void sigintHandler(){
    exit(0);
}
void handleMessage(struct ping_msg);

int main(int argc, char** argv){
    srand(time(NULL));

    atexit(&stop);
    signal(SIGINT, &sigintHandler);
    openQueue();
    if(connectToServer() == -1){
        puts("Couldn't connect to server");
        msgctl(mqPrivateID, IPC_RMID, NULL);
        exit(-1);
    }
    else{
        char* command = (char*)malloc(MAX_MSG*sizeof(char));
        while(1){
            // First, check my queue
            struct ping_msg recveivedData;
            int msgGot = msgrcv(mqPrivateID, &recveivedData, MAX_MSG, 0, IPC_NOWAIT);
            if(msgGot != -1){
                handleMessage(recveivedData);
            }
            else{
                scanf("%s", command);
                if(strcmp(command, "LIST") == 0)
                    list();
                else if(strcmp(command, "CONNECT") == 0){
                    int chatID;
                    scanf("%d", &chatID);
                    requestConnect(chatID);
                }
                else if(strcmp(command, "DISCONNECT") == 0){
                    requestDisconnect();
                }
                else if(strcmp(command, "SEND") == 0){
                    scanf("%s", command);
                    sendMessage(command);
                }
                else if(strcmp(command, "STOP") == 0)
                    exit(0);
                else if(strcmp(command, "CHECK") == 0)
                    ;
                else
                    puts("Invalid command");
            }
        }
        return 0;
    }
}

// BASIC ACTIONS TAKEN BY PROGRAM
void openQueue(){
    char* homePath = getenv("HOME");
    do{
        mqPrivateKey = ftok(homePath, rand());
        mqPrivateID = msgget(mqPrivateKey, IPC_CREAT | IPC_EXCL | S_IRWXU);
    }while(mqPrivateID == -1);
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

void stop(){
    struct clientid_msg stopMsg = {.mtype = STOP, .clientID = clientID};
    msgsnd(mqServerID, &stopMsg, sizeof(int), 0);
    msgctl(mqPrivateID, IPC_RMID, NULL);
    exit(0);
}

//////////////////////////////////////////

// REQUESTS POSSIBLE BY SPECYFING COMMANDS

void requestConnect(int connectClientID){
    struct connect_request_msg request = {.mtype = CONNECT, .requestClientID = clientID, .connectClientID = connectClientID};
    msgsnd(mqServerID, &request, 2*sizeof(int), 0);
    struct connect_msg answer;
    msgrcv(mqPrivateID, &answer, 2*sizeof(int), CONNECT_RESPONSE, 0);
    connectResponseHandler(&answer);
}

void requestDisconnect(){
    struct connect_request_msg msg = {.mtype = DISCONNECT, .connectClientID = clientID, .requestClientID = privateChatID};
    msgsnd(mqServerID, &msg, 2*sizeof(int), 0);
    struct clientid_msg answer;
    msgrcv(mqPrivateID, &answer, sizeof(int), DISCONNECT_RESPONSE, 0);
    disconnectResponseHandler();
}

void sendMessage(char* message){
    if(privateChatID == -1){
        puts("No private chat available");
    }
    else{
        printf("[--Me--]:\t%s\n===\n", message);
        struct ping_msg msg;
        msg.mtype = MESSAGE;
        strcpy(msg.rest, message);
        msgsnd(mqPrivateChatID, &msg, MAX_MSG, 0);
    }
}

void list(){
    struct clientid_msg idMsg = {.mtype = LIST, .clientID = clientID};
    msgsnd(mqServerID, &idMsg, sizeof(int), 0);

    struct list_cell cellRecieved;
    msgrcv(mqPrivateID, &cellRecieved, sizeof(int) + sizeof(client_state), LIST_CLIENT, 0);
    while(cellRecieved.clientID != -1){
        printf("ClientID: %d\t State: %s\n", cellRecieved.clientID, stateName(cellRecieved.clientState));
        msgrcv(mqPrivateID, &cellRecieved, sizeof(int) + sizeof(client_state), LIST_CLIENT, 0);
    }
    puts("------------------");
}

/////////////////////////////////////

// HANDLING INCOMING CONNECT REQUESTS

void connectResponseHandler(struct connect_msg* connect){
    if(connect->connectClientID != -1){
        privateChatID = connect->connectClientID;
        mqPrivateChatID = msgget(connect->connectClientKey, 0);
        printf("Connected to client %d\n", privateChatID);
    }
    else{
        puts("Connecting failed");
    }
}

void disconnectResponseHandler(){
    privateChatID = -1;
    mqPrivateChatID = -1;
    puts("Disconnected from private chat");
}

//////////////////////////////////////
// HANDLING INCOMING MESSAGES IN QUEUE
void handleMessage(struct ping_msg msg){
    switch (msg.mtype)
    {
    case SERVER_STOP:
        raise(SIGINT);
        break;
    case MESSAGE:
        printf("[--%d--]:\t%s\n===\n", privateChatID, msg.rest);
        break;
    case CONNECT_RESPONSE:
        connectResponseHandler((struct connect_msg*)&msg);
        break;
    case DISCONNECT_RESPONSE:
        disconnectResponseHandler();
        break;
    default:
        break;
    }
}