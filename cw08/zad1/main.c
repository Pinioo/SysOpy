#include "utils.h"

struct PGM pgmImg;
const int maxGray = 256;

int numOfThreads;
long** threadResults;
long* finalResults;
long* microsecTimes;

pthread_t* threadIds;

void* signThread(void* threadNumVoid){
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    //////////////////////////////
    int threadNumber = *(int*)threadNumVoid;
    free(threadNumVoid);
    int minGrayValue = (threadNumber * maxGray) / numOfThreads;
    int maxGrayValue = ((threadNumber + 1) * maxGray) / numOfThreads - 1;
    for(size_t i = 0; i < pgmImg.height; ++i) {
        for (size_t j = 0; j < pgmImg.width; ++j) {
            unsigned char value = pgmImg.board[i][j];
            if(value >= minGrayValue && value <= maxGrayValue)
                ++finalResults[value];
        }
    }
    //////////////////////////////

    struct timespec stop;
    clock_gettime(CLOCK_REALTIME, &stop);
    long nanos = stop.tv_nsec - start.tv_nsec;
    return (void*)(nanos/1000);
}

void* blockThread(void* threadNumVoid){
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    //////////////////////////////
    int threadNumber = *(int*)threadNumVoid;
    free(threadNumVoid);
    int firstColumn = (threadNumber * pgmImg.width) / numOfThreads;
    int lastColumn = ((threadNumber + 1) * pgmImg.width) / numOfThreads - 1;
    for(size_t row = 0; row < pgmImg.height; ++row){
        for(size_t col = firstColumn; col <= lastColumn; ++col){
            ++threadResults[threadNumber][pgmImg.board[row][col]];
        }
    }
    //////////////////////////////

    struct timespec stop;
    clock_gettime(CLOCK_REALTIME, &stop);
    long nanos = stop.tv_nsec - start.tv_nsec;
    return (void*)(nanos/1000);
}

void* interThread(void* threadNumVoid){
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    //////////////////////////////
    int threadNumber = *(int*)threadNumVoid;
    free(threadNumVoid);
    for(size_t row = 0; row < pgmImg.height; ++row){
        for(size_t col = threadNumber; col < pgmImg.width; col += numOfThreads){
            ++threadResults[threadNumber][pgmImg.board[row][col]];
        }
    }
    //////////////////////////////

    struct timespec stop;
    clock_gettime(CLOCK_REALTIME, &stop);
    long nanos = stop.tv_nsec - start.tv_nsec;
    return (void*)(nanos/1000);
}

int main(int argc, char** argv){
    if(argc < 4){
        puts("Too few arguments");
        exit(-1);
    }
    char* partitionMode = argv[1];
    numOfThreads = atoi(argv[2]);
    char* pgmFilename = argv[3];

    pgmImg = loadPgm(pgmFilename);
    finalResults = (long*)calloc(maxGray, sizeof(long));
    threadIds = (pthread_t*)malloc(numOfThreads*sizeof(pthread_t));
    microsecTimes = (long*)malloc(numOfThreads*sizeof(long));

    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);

    void* func;
    bool createThreadResultsArray;
    if(strcmp(partitionMode, "sign") == 0){
        createThreadResultsArray = false;
        func = signThread;
    }
    else if(strcmp(partitionMode, "block") == 0){
        createThreadResultsArray = true;
        func = blockThread;
    }
    else if(strcmp(partitionMode, "interleaved") == 0){
        createThreadResultsArray = true;
        func = interThread;
    }
    else {
        puts("Given partitioning mode is invalid");
        exit(-1);
    }

    if(createThreadResultsArray){
        threadResults = (long**)malloc(numOfThreads*sizeof(long*));
        for (size_t i = 0; i < numOfThreads; ++i)
            threadResults[i] = (long*)calloc(maxGray, sizeof(long));
    }
    
    for(int i = 0; i < numOfThreads; ++i){
        int* number = (int*)malloc(sizeof(int));
        *number = i;
        pthread_create(&threadIds[i], NULL, func, (void*)number);
    }

    for(int i = 0; i < numOfThreads; ++i){
        void* micros;
        pthread_join(threadIds[i], &micros);
        microsecTimes[i] = (long)micros;
    }

    if(createThreadResultsArray) {
        for (size_t i = 0; i < numOfThreads; ++i) {
            for (size_t j = 0; j < maxGray; ++j)
                finalResults[j] += threadResults[i][j];
            free(threadResults[i]);
        }
        free(threadResults);
    }

    struct timespec stop;
    clock_gettime(CLOCK_REALTIME, &stop);
    long fullMicrosTime = (stop.tv_nsec - start.tv_nsec)/1000;

    for (size_t i = 0; i < numOfThreads; ++i) {
        printf("ID: %lu TIME: %ldms\n", threadIds[i], microsecTimes[i]);
    }

    printf("PROGRAM EXECUTION TIME: %ldms\n", fullMicrosTime);
    
    char name[50];
    sprintf(name, "hist_%s_%s_%d.txt", pgmFilename, partitionMode, numOfThreads);
    saveHistogramToFile(finalResults, maxGray, name);

    for(int i = 0; i < pgmImg.height; ++i)
        free(pgmImg.board[i]);
    free(pgmImg.board);
    free(finalResults);
    free(microsecTimes);
    return 0;
}