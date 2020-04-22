#ifndef CHAT_FALGS

#include <stdlib.h>

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5
#define MAX_CLIENT_MTYPE INIT+1


#define SERVER_STOP 101
#define ID_RESPONSE 201
#define LIST_CLIENT 301
#define LIST_END 302
#define MAX_SERVER_MTYPE LIST_END+1


#define MAX_CLIENTS 2
#define MAX_MSG 1024

#define PROJ_ID 71831

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
};

#define CHAT_FALGS
#endif //CHAT_FLAGS