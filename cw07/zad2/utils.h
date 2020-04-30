#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/unistd.h>

#define MAX_ORDERS 50

#define W_PACKERS 10
#define W_RECEIVERS 15
#define W_SENDERS 5

#define ARRAY_NAME "my_ord_arr0"
#define ORDERS_INFO_NAME "my_ord_info0"
#define SEMAPHORE_NAME "my_sem0"

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
    int ordId = shm_open(ORDERS_INFO_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
    if(ordId == -1){
        shm_unlink(ORDERS_INFO_NAME);
        ordId = shm_open(ORDERS_INFO_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
    }
    ftruncate(ordId, sizeof(struct OrdersInfo));
    struct OrdersInfo* ordInfo = mmap(NULL, sizeof(struct OrdersInfo), PROT_WRITE, MAP_SHARED, ordId, 0);
    ordInfo->lastAddedIndex = -1; 
    ordInfo->lastSentIndex = -1;
    ordInfo->lastPackedIndex = -1;
    ordInfo->toPrepare = 0;
    ordInfo->toSend = 0;
    munmap(ordInfo, sizeof(struct OrdersInfo));
    return ordId;
}

int initOrdersArray(){
    int arrayId = shm_open(ARRAY_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
    if(arrayId == -1){
        shm_unlink(ARRAY_NAME);
        arrayId = shm_open(ARRAY_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
    }
    ftruncate(arrayId, MAX_ORDERS*sizeof(struct Order));
    struct Order* ordArray = mmap(NULL, MAX_ORDERS*sizeof(struct Order), PROT_WRITE, MAP_SHARED, arrayId, 0);
    for(int i = 0; i < MAX_ORDERS; ++i){
        ordArray[i].size = 0; 
        ordArray[i].packed = 0;
    }
    munmap(ordArray, MAX_ORDERS*sizeof(struct Order));
    return arrayId;
}

struct Order* getOrdersArray(){
    int desc = shm_open(ARRAY_NAME, O_RDWR, 0666);
    ftruncate(desc, MAX_ORDERS*sizeof(struct Order));
    return (struct Order*)mmap(NULL, MAX_ORDERS*sizeof(struct Order), PROT_READ | PROT_WRITE, MAP_SHARED, desc, 0);
}

struct OrdersInfo* getOrdersInfo(){
    int desc = shm_open(ORDERS_INFO_NAME, O_RDWR, 0666);
    ftruncate(desc, sizeof(struct OrdersInfo));
    return (struct OrdersInfo*)mmap(NULL, sizeof(struct OrdersInfo), PROT_READ | PROT_WRITE, MAP_SHARED, desc, 0);
}

void printPidTimestamp(){
    struct timeval time;
    gettimeofday(&time, NULL);
    int miliseconds = time.tv_usec / 1000;
    char buff[10];
  
    strftime(buff, 9, "%H:%M:%S", localtime(&time.tv_sec));
    printf("[%d %s:%d]", getpid(), buff, miliseconds);
}

sem_t* createSemaphore(){ 
    return sem_open(SEMAPHORE_NAME, O_RDWR | O_CREAT | O_EXCL, 0666, 1);
}

sem_t* getSemaphore(){
    return sem_open(SEMAPHORE_NAME, O_RDWR);
}

void lockSemaphore(sem_t* sem){
    sem_wait(sem);
}

void unlockSemaphore(sem_t* sem){
    sem_post(sem);
}

void closeSemaphore(sem_t* sem){
    sem_close(sem);
}

void deleteSemaphore(){
    sem_unlink(SEMAPHORE_NAME);
}

int nextIndex(int i){
    return (i+1) % MAX_ORDERS;
}