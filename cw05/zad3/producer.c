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
    
    FILE* file = fopen(filename, "r");
    FILE* pipe = fopen(pipename, "w");
    pid_t my_pid = getpid();

    int prefix_len = 2;
    for(int i = 1; my_pid / i > 0; i *= 10)
        ++prefix_len;

    char* prefix = malloc((prefix_len + 1)*sizeof(char));
    sprintf(prefix, "#%d#", my_pid);

    char* read_part = (char*)malloc((N + 1)*sizeof(char));
    char* to_write = (char*)malloc((N + prefix_len + 1)*sizeof(char));
    
    while(!feof(file)){
        int chars = fread(read_part, 1, N, file);
        sprintf(to_write, "%s%s", prefix, read_part);
        fwrite(to_write, 1, prefix_len + chars, pipe);
        fflush(pipe);
        sleep(1);
    }
    free(read_part);
    fclose(file);
    fclose(pipe);
    return 0;
}