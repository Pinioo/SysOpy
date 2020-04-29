#include "utils.h"

int semId;
int arrayId;
int infoId;
struct OrdersInfo* info;
struct Order* array;
pid_t children[W_PACKERS + W_RECEIVERS + W_SENDERS];

void sigIntActions(){
    for(int i = 0; i < W_PACKERS + W_RECEIVERS + W_SENDERS; ++i){
        kill(SIGINT, children[i]);
    }
}

void exitActions(){
    semctl(semId, 0, IPC_RMID, NULL);

    shmdt(info);
    shmctl(infoId, IPC_RMID, NULL);

    shmdt(array);
    shmctl(arrayId, IPC_RMID, NULL);
}

int main(int argc, char** argv){
    semId = createSemaphore();
    infoId = initOrdersInfo();
    arrayId = initOrdersArray();

    info = getOrdersInfo();
    array = getOrdersArray();

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