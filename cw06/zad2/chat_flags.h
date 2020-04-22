#ifndef CHAT_FALGS

#include <stdlib.h>

#define SERVER_MQ_NAME "/servqueue"

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define MESSAGE 6
#define MAX_CLIENT_MTYPE (MESSAGE+1)

#define SERVER_STOP 11

#define ID_RESPONSE 21

#define LIST_CLIENT 31

#define CONNECT_RESPONSE 41
#define DISCONNECT_RESPONSE 42
#define MAX_SERVER_MTYPE DISCONNECT_RESPONSE+1

#define MAX_CLIENTS 64
#define MAX_MSG 1024
#define MAX_MSGS_IN 10

#define MQ_NAME_LEN 14

typedef enum client_state{
    NEXIST, ACTIVE, PRIVATE_CONNECT
} client_state;

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