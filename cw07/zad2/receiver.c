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

        int nextAddCand = nextIndex(ordInfo->lastAddedIndex);
        if(ordArray[nextAddCand].size == 0){
            ordArray[nextAddCand].size = (rand() % 100) + 1;
            ordArray[nextAddCand].packed = 0;
            ++ordInfo->toPrepare;
            ordInfo->lastAddedIndex = nextAddCand;
            printPidTimestamp();
            printf(" Added: %d, To Pack: %d, To Send: %d\n", ordArray[nextAddCand].size, ordInfo->toPrepare, ordInfo->toSend);
        } 
        unlockSemaphore(semaphore);
    }
    return 0;
}