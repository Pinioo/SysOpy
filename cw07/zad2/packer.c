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
        unlockSemaphore(semaphore);
    }
    return 0;
}