#include "utils.h"

int semId;
struct Order* ordArray;
struct OrdersInfo* ordInfo;

void exitActions(){
    shmdt(ordArray);
    shmdt(ordInfo);
}

int main(){
    ordInfo = getOrdersInfo();
    ordArray = getOrdersArray();
    semId = getSemaphore();

    atexit(exitActions);

    while(1){
        lockSemaphore(semId);

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
        unlockSemaphore(semId);
    }
    return 0;
}