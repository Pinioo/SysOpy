#include "utils.h"

int nextPart(FILE* file){
    int number;
    while(fscanf(file, "#%*[^\n]\n") > 0);
    fscanf(file, "%d", &number);
    return number;
}

struct PGM loadPgm(const char* filename){
    FILE* file = fopen(filename, "r");
    struct PGM loadedImg;
    char* line = (char*)malloc(1000*sizeof(char));
    size_t size;

    do {
        getline(&line, &size, file); 
    } while(line[0] == '#');

    if (line[0] != 'P' || line[1] != '2'){
        puts("File isn't PGM format");
        exit(-1);
    }
    loadedImg.width = nextPart(file);

    loadedImg.height = nextPart(file);

    loadedImg.maxGray = nextPart(file);

    loadedImg.board = (unsigned char**)malloc(loadedImg.height*sizeof(unsigned char*));


    for (size_t i = 0; i < loadedImg.height; ++i) {
        loadedImg.board[i] = (unsigned char*)malloc(loadedImg.width*sizeof(unsigned char));
        for (size_t j = 0; j < loadedImg.width; ++j) {
            loadedImg.board[i][j] = (unsigned char)nextPart(file);
        }
    }
    fclose(file);

    return loadedImg;
}

void saveHistogramToFile(long* results, int maxGray, const char* filename){
    FILE* file = fopen(filename, "w");
    for(size_t i = 0; i < maxGray; ++i){
        fprintf(file, "%ld %ld\n", i, results[i]);
    }
    fclose(file);
}