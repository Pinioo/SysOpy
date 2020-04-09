#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>

int usr1_caught = 0;
int sigs_to_send;
pid_t catcher_pid;

void do_nothing (int s) {}

void usr1_reaction(){
    kill(catcher_pid, SIGUSR1);
    ++usr1_caught;
}

void usr2_reaction(){
    printf("Sender caught %d/%d signals\n", usr1_caught, sigs_to_send);
    exit(0);
}

int main(int argc, char** argv){
    printf("PID: %d\n", getpid());
    if(argc < 3){
        printf("Too few arguments\n");
        exit(-1);
    }

    catcher_pid = atoi(argv[1]);
    sigs_to_send = atoi(argv[2]);
    
    sigset_t tmp_mask;
    sigset_t full_mask;

    sigfillset(&full_mask);       
    sigfillset(&tmp_mask);

    sigdelset(&tmp_mask, SIGUSR1);

    struct sigaction tmp_sa = {.sa_handler = &do_nothing, .sa_flags = 0};
    
    sigaction(SIGUSR1, &tmp_sa, NULL);
    sigprocmask(SIG_BLOCK, &full_mask, NULL);
    
    for(int i = 0; i < sigs_to_send; ++i){
        kill(catcher_pid, SIGUSR1);
        sigsuspend(&tmp_mask);
    } 
    
    kill(catcher_pid, SIGUSR2);

    struct sigaction sa_usr1 = {.sa_handler = &usr1_reaction, .sa_flags = 0};
    struct sigaction sa_usr2 = {.sa_handler = &usr2_reaction, .sa_flags = 0};

    sigfillset(&sa_usr1.sa_mask);
    sigfillset(&sa_usr2.sa_mask);
    
    sigaction(SIGUSR1, &sa_usr1, NULL);
    sigaction(SIGUSR2, &sa_usr2, NULL);

    sigdelset(&tmp_mask, SIGUSR2);
    kill(catcher_pid, SIGUSR2);

    while(1){
        sigsuspend(&tmp_mask);
    }
    return 0;
}