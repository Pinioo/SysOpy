#include "utils.h"

sem_t* semaphore;
// int infoId;
// int arrayId;
// struct OrdersInfo* info;
// struct Order* array;
pid_t children[W_PACKERS + W_RECEIVERS + W_SENDERS];

void sigIntActions(){
    for(int i = 0; i < W_PACKERS + W_RECEIVERS + W_SENDERS; ++i){
        kill(SIGINT, children[i]);
    }
}

void exitActions(){
    closeSemaphore(semaphore);
    deleteSemaphore();

    shm_unlink(ARRAY_NAME);

    shm_unlink(ORDERS_INFO_NAME);
}

int main(int argc, char** argv){
    semaphore = createSemaphore();
    initOrdersInfo();
    initOrdersArray();

    signal(SIGINT, sigIntActions);
    atexit(exitActions);

    size_t childIndex = 0;
    pid_t childId;
    for(size_t i = 0; i < W_RECEIVERS; ++i){
        if((childId = fork()) == 0)
            execl("./receiver", "./receiver", NULL);
        else
            children[childIndex++] = childId; 
    }
    for(size_t i = 0; i < W_PACKERS; ++i){
        if((childId = fork()) == 0)
            execl("./packer", "./packer", NULL);
        else
            children[childIndex++] = childId;
    }
    for(size_t i = 0; i < W_SENDERS; ++i){
        if((childId = fork()) == 0)
            execl("./sender", "./sender", NULL);
        else
            children[childIndex++] = childId;
    }
    while (wait(NULL) > 0);
    return 0;
}