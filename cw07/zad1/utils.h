#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/unistd.h>

#define MAX_ORDERS 50

#define W_PACKERS 10
#define W_RECEIVERS 15
#define W_SENDERS 5

#define ARRAY_KEY (ftok(getenv("HOME"), 0))
#define ORDERS_INFO_KEY (ftok(getenv("HOME"), 1))
#define SEMAPHORE_KEY (ftok(getenv("HOME"), 2))

union semun {
    int val;
    struct semid_ds* buf;
    int* array;
    struct seminfo *__buf;
};

struct OrdersInfo {
    int lastAddedIndex;
    int lastSentIndex;
    int lastPackedIndex;
    int toPrepare;
    int toSend;
};

struct Order {
    int size;
    int packed;
};

int initOrdersInfo(){
    int infoId = shmget(ORDERS_INFO_KEY, sizeof(struct OrdersInfo), 0666 | IPC_CREAT);
    struct OrdersInfo* ordInfo = shmat(infoId, NULL, 0);
    ordInfo->lastAddedIndex = -1; 
    ordInfo->lastSentIndex = -1;
    ordInfo->lastPackedIndex = -1;
    ordInfo->toPrepare = 0;
    ordInfo->toSend = 0;
    return infoId;
}

int initOrdersArray(){
    int arrayId = shmget(ARRAY_KEY, MAX_ORDERS*sizeof(struct Order), 0666 | IPC_CREAT);
    struct Order* ordArray = shmat(arrayId, NULL, 0);
    for(int i = 0; i < MAX_ORDERS; ++i){
        ordArray[i].size = 0; 
        ordArray[i].packed = 0;
    }
    return arrayId;
}

struct Order* getOrdersArray(){
    int id = shmget(ARRAY_KEY, 0, 0666);
    return (struct Order*)shmat(id, NULL, 0);
}

struct OrdersInfo* getOrdersInfo(){
    int id = shmget(ORDERS_INFO_KEY, 0, 0666);
    return (struct OrdersInfo*)shmat(id, NULL, 0);
}

void printPidTimestamp(){
    struct timeval time;
    gettimeofday(&time, NULL);
    int miliseconds = time.tv_usec / 1000;
    char buff[10];
  
    strftime(buff, 9, "%H:%M:%S", localtime(&time.tv_sec));
    printf("[%d %s:%d]", getpid(), buff, miliseconds);
}

int createSemaphore(){
    int id = semget(SEMAPHORE_KEY, 1, 0666 | IPC_CREAT);

    union semun arg;
    arg.val = 1;
    semctl(id, 0, SETVAL, arg);

    return id;
}

int getSemaphore(){
    return semget(SEMAPHORE_KEY, 1, 0);
}

void lockSemaphore(int semId){
    struct sembuf buf = {
        .sem_num = 0,
        .sem_op = -1,
        .sem_flg = 0
    };
    
    semop(semId, &buf, 1);
}

void unlockSemaphore(int semId){
    struct sembuf buf = {
        .sem_num = 0,
        .sem_op = 1,
        .sem_flg = 0
    };
    
    semop(semId, &buf, 1);
}

int nextIndex(int i){
    return (i+1) % MAX_ORDERS;
}