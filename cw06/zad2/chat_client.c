#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mqueue.h>
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


mqd_t mqPrivateID;
mqd_t mqServerID;
mqd_t mqPrivateChatID;

struct mq_attr attr = {.mq_maxmsg = MAX_MSGS_IN, .mq_msgsize = MAX_MSG, .mq_flags = 0};
struct mq_attr nonBlockAttr = {.mq_maxmsg = MAX_MSGS_IN, .mq_msgsize = MAX_MSG, .mq_flags = O_NONBLOCK};

char mqPrivateName[MQ_NAME_LEN + 1];

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
void handleMessage(char*);

int main(int argc, char** argv){
    srand(time(NULL));

    atexit(&stop);
    signal(SIGINT, &sigintHandler);
    openQueue();
    if(connectToServer() == -1){
        puts("Couldn't connect to server");
        mq_close(mqPrivateID);
        mq_unlink(mqPrivateName);
        exit(-1);
    }
    else{
        char* command = (char*)malloc(MAX_MSG*sizeof(char));
        while(1){
            // First, check my queue
            char recveivedData[MAX_MSG];
            mq_setattr(mqPrivateID, &nonBlockAttr, NULL);
            int msgGot = mq_receive(mqPrivateID, recveivedData, MAX_MSG, NULL);
            mq_setattr(mqPrivateID, &attr, NULL);
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
    mqPrivateName[0] = '/';
    mqPrivateName[MQ_NAME_LEN] = '\0';
    do{
        for(size_t i = 1; i < MQ_NAME_LEN; ++i)
            mqPrivateName[i] = (char)('a' + rand() % ('z'- 'a'));
    }while((mqPrivateID = mq_open(mqPrivateName, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, &attr)) == -1);
}

int connectToServer(){
    mqServerID = mq_open(SERVER_MQ_NAME, O_WRONLY);

    char initMsg[MAX_MSG];
    initMsg[0] = INIT;
    for(int i = 0; i < MQ_NAME_LEN + 1; ++i)
        initMsg[i + 1] = mqPrivateName[i]; 
    mq_send(mqServerID, initMsg, MAX_MSG, 0);

    char response[MAX_MSG];
    do{
        mq_receive(mqPrivateID, response, MAX_MSG, NULL);
    }while(response[0] != ID_RESPONSE);
    clientID = (int)response[1];
    if(clientID != MAX_CLIENTS)
        return 0;
    else
        return -1;
}

void stop(){
    // struct clientid_msg stopMsg = {.mtype = STOP, .clientID = clientID};
    char stopMsg[MAX_MSG] = {STOP, clientID};
    mq_send(mqServerID, stopMsg, MAX_MSG, 0);
    mq_close(mqPrivateID);
    mq_unlink(mqPrivateName);
    exit(0);
}

//////////////////////////////////////////

// REQUESTS POSSIBLE BY SPECYFING COMMANDS

void requestConnect(int connectClientID){
    char request[MAX_MSG] = {CONNECT, clientID, connectClientID};
    mq_send(mqServerID, request, MAX_MSG, 0);
    char answer[MAX_MSG];
    do{
        mq_receive(mqPrivateID, answer, MAX_MSG, NULL);
    }while(answer[0] != CONNECT_RESPONSE);
    connectResponseHandler(&answer[1]);
}

void requestDisconnect(){
    char request[MAX_MSG] = {DISCONNECT, clientID, privateChatID};
    mq_send(mqServerID, request, MAX_MSG, 0);
    char answer[MAX_MSG];
    do{
        mq_receive(mqPrivateID, answer, MAX_MSG, NULL);
    }while(answer[0] != DISCONNECT_RESPONSE);
    disconnectResponseHandler();
}

void sendMessage(char* message){
    if(privateChatID == -1){
        puts("No private chat available");
    }
    else{
        printf("[--Me--]:\t%s\n===\n", message);
        char msg[MAX_MSG];
        msg[0] = MESSAGE;
        strcpy(&msg[1], message);
        mq_send(mqPrivateChatID, msg, MAX_MSG, 0);
    }
}

void list(){
    char idMsg[MAX_MSG] = {LIST, clientID};
    mq_send(mqServerID, idMsg, MAX_MSG, 0);

    char cellReceived[MAX_MSG];
    mq_setattr(mqPrivateID, &attr, NULL);
    mq_receive(mqPrivateID, cellReceived, MAX_MSG, NULL);
    while(cellReceived[1] != -1){
        if(cellReceived[0] == LIST_CLIENT)
            printf("ClientID: %d\t State: %s\n", cellReceived[1], stateName(cellReceived[2]));
        mq_receive(mqPrivateID, cellReceived, MAX_MSG, NULL);
    }
    puts("------------------");
}

/////////////////////////////////////

// HANDLING INCOMING CONNECT REQUESTS

void connectResponseHandler(char* connect){
    if(connect[0] != -1){
        privateChatID = connect[0];
        mqPrivateChatID = mq_open(&connect[1], O_WRONLY);
    }
    if(connect[0] == -1 || mqPrivateChatID == -1){
        puts("Connecting failed");
    }
    else{
        printf("Connected to client %d\n", privateChatID);
    }
}

void disconnectResponseHandler(){
    privateChatID = -1;
    mqPrivateChatID = -1;
    puts("Disconnected from private chat");
}

//////////////////////////////////////
// HANDLING INCOMING MESSAGES IN QUEUE
void handleMessage(char* msg){
    switch (msg[0])
    {
    case SERVER_STOP:
        raise(SIGINT);
        break;
    case MESSAGE:
        printf("[--%d--]:\t%s\n===\n", privateChatID, &msg[1]);
        break;
    case CONNECT_RESPONSE:
        connectResponseHandler(&msg[1]);
        break;
    case DISCONNECT_RESPONSE:
        disconnectResponseHandler();
        break;
    default:
        break;
    }
}