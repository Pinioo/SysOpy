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

        int nextAddCand = nextIndex(ordInfo->lastAddedIndex);
        if(ordArray[nextAddCand].size == 0){
            ordArray[nextAddCand].size = (rand() % 100) + 1;
            ordArray[nextAddCand].packed = 0;
            ++ordInfo->toPrepare;
            ordInfo->lastAddedIndex = nextAddCand;
            printPidTimestamp();
            printf(" Added: %d, To Pack: %d, To Send: %d\n", ordArray[nextAddCand].size, ordInfo->toPrepare, ordInfo->toSend);
        } 
        unlockSemaphore(semId);
    }
    return 0;
}