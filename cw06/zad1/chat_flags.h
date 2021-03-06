#ifndef CHAT_FALGS

#include <stdlib.h>

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define MESSAGE 6
#define MAX_CLIENT_MTYPE (MESSAGE+1)

#define SERVER_STOP 101

#define ID_RESPONSE 201

#define LIST_CLIENT 301

#define CONNECT_RESPONSE 401
#define DISCONNECT_RESPONSE 402
#define MAX_SERVER_MTYPE DISCONNECT_RESPONSE+1

#define MAX_CLIENTS 64
#define MAX_MSG 1024

#define PROJ_ID 71831

typedef enum client_state{
    NEXIST, ACTIVE, PRIVATE_CONNECT
} client_state;

struct clientid_msg {
    long mtype;
    int clientID;
};

struct ping_msg {
    long mtype;
    char rest[MAX_MSG];
};

struct clientkey_msg {
    long mtype;
    key_t clientKey;
};

struct list_cell{
    long mtype;
    int clientID;
    client_state clientState;
};

struct connect_request_msg{
    long mtype;
    int requestClientID;
    int connectClientID;
};

struct connect_msg{
    long mtype;
    int connectClientID;
    key_t connectClientKey;
};

const char* stateName(client_state state){
    switch(state){
        case NEXIST:
            return "Do not exist";
        
        case ACTIVE:
            return "ACTIVE";

        case PRIVATE_CONNECT:
            return "PRIVATE_CONNECT";

        default:
            return "Err";
    }
}

#define CHAT_FALGS
#endif //CHAT_FLAGS