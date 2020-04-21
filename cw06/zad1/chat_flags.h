#ifndef CHAT_FALGS

#include <stdlib.h>

#define STOP 1
#define DISCONNECT 2
#define LIST 3
#define CONNECT 4
#define INIT 5

#define ID_RESPONSE 20

#define MAX_CLIENTS 256
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

#define CHAT_FALGS
#endif //CHAT_FLAGS