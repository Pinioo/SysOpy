#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>

int stopped = 0;

void sigtstp_reaction(){
    stopped ^= 1;
    if(stopped){
        printf("Waiting for CTRL+Z - continue or CTRL+C - exit\n");
    }
}

void sigint_reaction(){
    printf("SIGINT caught\n");
    exit(0);
}

int main(){
    sigset_t full_mask;
    sigfillset(&full_mask);

    struct sigaction sa_int = {.sa_handler = &sigint_reaction, .sa_flags = 0, .sa_mask = full_mask};
    struct sigaction sa_tstp = {.sa_handler = &sigtstp_reaction, .sa_flags = 0, .sa_mask = full_mask};
    
    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGTSTP, &sa_tstp, NULL);

    sigset_t prog_mask;
    sigfillset(&prog_mask);
    sigdelset(&prog_mask, SIGINT);
    sigdelset(&prog_mask, SIGTSTP);
    sigprocmask(SIG_BLOCK, &prog_mask, NULL);

    while(1){
        sleep(1);
        if(stopped){
            pause();
        }
        system("ls");
    }
    return 0;
}