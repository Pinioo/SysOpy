#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

// argv[1] = ignore, handler, mask or pending

void handler(){
    printf("SIGUSR1 caught by process %d\n", getpid());
}

void mask_handler(){
    printf("SIGUSR1 caught by process %d\n", getpid());
}

int main(int argc, char** argv){
    printf("Parent PID: %d\n", getpid());
    struct sigaction sa; 
    int check_if_pending = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = NULL;

    if(argc < 2){
        printf("Too few arguments\n");
        exit(-1);
    }

    if(strcmp(argv[1], "ignore") == 0){
        sa.sa_handler = SIG_IGN;
    } else if(strcmp(argv[1], "handler") == 0){
        sa.sa_handler = &handler;
    } else if(strcmp(argv[1], "mask") == 0){
        sigaddset(&sa.sa_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
    } else if(strcmp(argv[1], "pending") == 0){
        sigaddset(&sa.sa_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
        check_if_pending = 1;
    } else {
        printf("Invalid action\n");
        exit(-1);
    }
    if(sa.sa_handler != NULL)
        sigaction(SIGUSR1, &sa, NULL);

    pid_t child;

    if(check_if_pending){
        printf("\n%d raises SIGUSR1\n", getpid());
        raise(SIGUSR1);
        child = fork();

        if(child != 0)
            wait(NULL);

        sigset_t pending;
        sigpending(&pending);
        if(sigismember(&pending, SIGUSR1))
            printf("SIGUSR1 is waiting in process %d\n", getpid());
        if(child == 0)
            exit(0);
    } else {
        child = fork();
        if(child == -1){
            printf("Fork wasn't successful\n");
            exit(-1);
        }
        if(child != 0)
            wait(NULL);
        
        printf("\n%d raises SIGUSR1\n", getpid());
        raise(SIGUSR1);

        if(child == 0)
            exit(0);
    }
    if(child != 0){
        execl("./sig_raiser", "./sig_raiser", argv[1], NULL);
    }
    return 0;
}