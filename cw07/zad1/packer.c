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

        int nextPackCand = nextIndex(ordInfo->lastPackedIndex);
        if(ordArray[nextPackCand].size != 0 && ordArray[nextPackCand].packed == 0){
            ordArray[nextPackCand].size *= 2;
            ordArray[nextPackCand].packed = 1;
            --ordInfo->toPrepare;
            ++ordInfo->toSend;
            ordInfo->lastPackedIndex = nextPackCand;
            printPidTimestamp();
            printf(" Prepared package of size: %d, To Pack: %d, To Send: %d\n", ordArray[nextPackCand].size, ordInfo->toPrepare, ordInfo->toSend);
        } 
        unlockSemaphore(semId);
    }
    return 0;
}