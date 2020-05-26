#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/unistd.h>

pthread_mutex_t chairsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t golarzCuttingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t golarzSleepingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientWaitingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientLeftMutex = PTHREAD_MUTEX_INITIALIZER;

// is golarz supposed to sleep condition
pthread_cond_t sleepingCond = PTHREAD_COND_INITIALIZER;
// is golarz Filip busy condition (cannot cut client's hair right now)
pthread_cond_t golarzCuttingCond = PTHREAD_COND_INITIALIZER;
// is client ready condition
pthread_cond_t clientWaitingCond = PTHREAD_COND_INITIALIZER;
// did client leave condition
pthread_cond_t clientLeftCond = PTHREAD_COND_INITIALIZER;

pthread_t* threads;

int clientsNumber;
int cutClientsNumber = 0;

int chairsNumber;
int chairsOccupied = 0;

int readyClientId = -1;
bool isGolarzSleeping = false;
bool isClientReady = false;
bool isGolarzCutting = true;


bool isAnyChairEmpty(){
    return chairsOccupied < chairsNumber;
}

bool isWaitingRoomEmpty(){
    return chairsOccupied == 0;
}

void golarzFilipWakeUp(){
    isGolarzSleeping = false;
    pthread_mutex_lock(&golarzSleepingMutex);
    pthread_cond_broadcast(&sleepingCond);
    pthread_mutex_unlock(&golarzSleepingMutex);
}

void* golarzFilipThreadActions(void* in){
    pthread_mutex_lock(&golarzSleepingMutex);
    pthread_mutex_lock(&clientWaitingMutex);
    pthread_mutex_lock(&clientLeftMutex);
    while(cutClientsNumber < clientsNumber){
        pthread_mutex_lock(&chairsMutex);
        while(isWaitingRoomEmpty()){
            // Sleeping phase
            pthread_mutex_unlock(&chairsMutex);
            puts("Golarz Filip: ide spac\n---");
            isGolarzSleeping = true;
            pthread_cond_wait(&sleepingCond, &golarzSleepingMutex);
            pthread_mutex_lock(&chairsMutex);
        }
        pthread_mutex_unlock(&chairsMutex);

        // Golarz is ready to cut; inform clients
        pthread_mutex_lock(&golarzCuttingMutex);
        isGolarzCutting = false;
        pthread_cond_broadcast(&golarzCuttingCond);
        pthread_mutex_unlock(&golarzCuttingMutex);

        // Golarz waiting for any client to be ready
        while(!isClientReady){
            pthread_cond_wait(&clientWaitingCond, &clientWaitingMutex);
        }

        printf("Golarz Filip strzyze klienta. ID: %d\n---\n", readyClientId);
        // Golarz is waiting for client to leave
        while(isClientReady){
            pthread_cond_wait(&clientLeftCond, &clientLeftMutex);
        }

        ++cutClientsNumber;
    }
    pthread_mutex_unlock(&golarzSleepingMutex);
    pthread_mutex_unlock(&clientWaitingMutex);
    pthread_mutex_unlock(&clientLeftMutex);
    return (void*)0;
}

void* clientThreadActions(void* in){
    int myId = *(int*)in;
    free(in);
    while(true){
        pthread_mutex_lock(&chairsMutex);
        if(isAnyChairEmpty())
            break;
        
        pthread_mutex_unlock(&chairsMutex);
        printf("Zajete. ID: %d\n---\n", myId);
        usleep(1000000 + rand() % 2000000);
    }
    ++chairsOccupied;
    pthread_mutex_unlock(&chairsMutex);

    pthread_mutex_lock(&golarzCuttingMutex);
    pthread_mutex_lock(&chairsMutex);
    if(isGolarzSleeping){
        printf("Budze golarza Filipa. ID: %d\n---\n", myId);
        golarzFilipWakeUp();
    }
    else{
        printf("Poczekalnia. %d wolne miejsca. ID: %d\n---\n", chairsNumber - chairsOccupied, myId);
    }
    pthread_mutex_unlock(&chairsMutex);
    while(isGolarzCutting){
        pthread_cond_wait(&golarzCuttingCond, &golarzCuttingMutex);
    }
    isGolarzCutting = true;
    pthread_mutex_unlock(&golarzCuttingMutex);

    // Client is ready to be cut; inform Golarz
    pthread_mutex_lock(&clientWaitingMutex);
    isClientReady = true;
    readyClientId = myId;
    pthread_cond_broadcast(&clientWaitingCond);
    pthread_mutex_unlock(&clientWaitingMutex);

    pthread_mutex_lock(&chairsMutex);
    --chairsOccupied;
    pthread_mutex_unlock(&chairsMutex);

    usleep(1000000 + rand()%(1000000));

    // Client's hair is perfectly cut, leave the room and inform Golarz
    printf("Wychodze. ID: %d\n---\n", myId);
    pthread_mutex_lock(&clientLeftMutex);
    isClientReady = false;
    pthread_cond_broadcast(&clientLeftCond);
    pthread_mutex_unlock(&clientLeftMutex);
    return (void*)0;
}

void cleanup(){
    pthread_cond_destroy(&clientWaitingCond);
    pthread_cond_destroy(&clientLeftCond);
    pthread_cond_destroy(&golarzCuttingCond);
    pthread_cond_destroy(&sleepingCond);

    pthread_mutex_destroy(&clientWaitingMutex);
    pthread_mutex_destroy(&clientLeftMutex);
    pthread_mutex_destroy(&golarzSleepingMutex);
    pthread_mutex_destroy(&golarzCuttingMutex);
    pthread_mutex_destroy(&chairsMutex);
}

int main(int argc, char** argv){
    // Golarz Filip has no time to clean his office, so he will do it at the end of the day
    atexit(cleanup);

    if(argc < 3){
        puts("Za malo argumentow");
        puts("Golarz Filip musi wiedziec ile ma postawic KRZESEL oraz ilu KLIENTOW ma obsluzyc");
        exit(-1);
    }
    chairsNumber = atoi(argv[1]);
    clientsNumber = atoi(argv[2]);
    if(clientsNumber < 1){
        puts("Wyglada na to, ze nikt nie przyjdzie dzis do zakladu");
        puts("Niezadowolony golarz Filip wraca do domu");
        exit(-1);
    }
    if(chairsNumber < 1){
        puts("Golarza Filip jest uczciwym czlowiekiem");
        puts("Nie otworzy zakladu nie majac krzesel w poczekalni");
        exit(-1);
    }
    printf(">>>> Golarz Filip otwiera zaklad\n>>>> W poczekalni %d krzesel, czekajacych na %d klientow\n---\n", chairsNumber, clientsNumber);

    srand(time(NULL));
    threads = (pthread_t*)malloc((clientsNumber + 1)*sizeof(pthread_t));

    // Golarz Filip starts a day of work
    pthread_create(&threads[0], NULL, golarzFilipThreadActions, (void*)0);
    
    // Clients are coming
    for(int i = 0; i < clientsNumber; ++i){
        usleep(1000000 + rand()%3000000);
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i+1], NULL, clientThreadActions, (void*)id);
    }

    // Now we check if every person (thread) left the building
    for(int i = 0; i < clientsNumber+1; ++i){
        pthread_join(threads[i], NULL);
    }

    return 0;
}