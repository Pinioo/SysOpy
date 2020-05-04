#ifndef UTILS_H_

#include <sys/file.h>
#include <sys/fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>

struct PGM{
    int width, height;
    int maxGray;
    unsigned char** board;
};

int nextPart(FILE* file);
void saveHistogramToFile(long* results, int maxGray, const char* filename);
struct PGM loadPgm(const char* filename);

#define UTILS_H_
#endif