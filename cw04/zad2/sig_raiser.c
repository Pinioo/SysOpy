#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv){
    if(argc < 2){
        printf("Too few arguments");
        exit(-1);
    }
    if(strcmp(argv[1], "pending") == 0){
        sigset_t pending;
        sigpending(&pending);
        if(sigismember(&pending, SIGUSR1))
            printf("SIGUSR1 is waiting in executed\n");
    }
    else{
        printf("\nExecuted raises SIGUSR1\n");
        raise(SIGUSR1);
    }
    return 0;
}