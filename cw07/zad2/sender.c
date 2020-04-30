#include "utils.h"

sem_t* semaphore;
struct Order* ordArray;
struct OrdersInfo* ordInfo;

void exitActions(){
    munmap(ordArray, MAX_ORDERS*sizeof(struct Order));
    munmap(ordInfo, sizeof(struct OrdersInfo));
}

int main(){
    ordInfo = getOrdersInfo();
    ordArray = getOrdersArray();
    semaphore = getSemaphore();

    atexit(exitActions);

    while(1){
        lockSemaphore(semaphore);

        int nextSendCand = nextIndex(ordInfo->lastSentIndex);
        if(ordArray[nextSendCand].packed == 1){
            ordArray[nextSendCand].size *= 3;
            --ordInfo->toSend;
            printPidTimestamp();
            printf(" Sent package of size: %d, To Pack: %d, To Send: %d\n", ordArray[nextSendCand].size, ordInfo->toPrepare, ordInfo->toSend);

            ordInfo->lastSentIndex = nextSendCand;
            ordArray[nextSendCand].size = 0;
            ordArray[nextSendCand].packed = 0;
        } 

        unlockSemaphore(semaphore);
    }
    return 0;
}