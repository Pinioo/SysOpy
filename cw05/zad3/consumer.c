#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char** argv){
    if(argc < 4){
        puts("Too few arguments");
        exit(-1);
    }
    char* pipename = argv[1];
    char* filename = argv[2];
    int N = atoi(argv[3]);
    
    FILE* file = fopen(filename, "w");
    FILE* pipe = fopen(pipename, "r");

    char* read_part = (char*)malloc(N*sizeof(char));
    
    while(!feof(pipe)){
        int chars = fread(read_part, 1, N, pipe);
        fwrite(read_part, 1, chars, file);
    }

    free(read_part);
    fclose(file);
    fclose(pipe);
    puts("Consumer end");
    return 0;
}