#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "chat_flags.h"

key_t mqPrivateKey;
key_t mqServerKey;

int mqPrivateID;
int mqServerID;

long clientID;

void stop();
void openQueue();
void connectWithServer();
void sendMessage(char*);

int main(int argc, char** argv){
    openQueue();

    return 0;
}

void openQueue(){
    char* homePath = getenv("HOME");
    mqServerKey = ftok(homePath, PROJ_ID);
    mqServerID = msgget(mqServerKey, 0);
    do{
        mqPrivateKey = ftok(homePath, PROJ_ID);

        // TODO: errno handle
        mqPrivateID = msgget(mqPrivateKey, IPC_CREAT | S_IRWXU);
    }while(0);
    struct clientkey_msg keyMsg = {.mtype = INIT, .clientKey = mqPrivateKey};
    msgsnd(mqServerID, &keyMsg, MAX_MSG, 0);

    struct ping_msg response;
    msgrcv(mqPrivateID, &response, MAX_MSG, ID_RESPONSE, 0);
    printf("Chat client ID: %d\n", ((struct clientid_msg*)&response)->clientID);
}