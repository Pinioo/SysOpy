#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <string.h>
#include <sys/types.h>

extern const char *const sys_siglist[];
int usr1_caught = 0;
int mode;
sigset_t tmp_mask;
sigset_t full_mask;

void placeholder(){}

void usr1_reaction(int sig, siginfo_t* info, void *ucontext){
    ++usr1_caught;
    kill(info->si_pid, SIGUSR1);
    printf("%d\n", usr1_caught);
}

void usr2_reaction(int sig, siginfo_t* info, void *ucontext){
    struct sigaction sa_tmp = {.sa_handler = &placeholder, .sa_flags = 0};
    
    sigaction(SIGUSR1, &sa_tmp, NULL);

    sigfillset(&tmp_mask);
    sigdelset(&tmp_mask, SIGUSR1);

    for(int i = 0; i < usr1_caught; ++i){
        kill(info->si_pid, SIGUSR1);
        sigsuspend(&tmp_mask);
    }
    kill(info->si_pid, SIGUSR2);
    exit(0);
}

int main(int argc, char** argv){
    if(argc < 1){
        printf("Too few arguments\n");
        exit(-1);
    }
    printf("PID: %d\n", getpid());

    sigfillset(&full_mask);
    sigprocmask(SIG_BLOCK, &full_mask, NULL);

    struct sigaction sa_usr1 = {.sa_sigaction = &usr1_reaction, .sa_flags = SA_SIGINFO, .sa_mask = full_mask};
    struct sigaction sa_usr2 = {.sa_sigaction = &usr2_reaction, .sa_flags = SA_SIGINFO, .sa_mask = full_mask};

    sigset_t mask_set;
    sigfillset(&mask_set);

    sigdelset(&mask_set, SIGUSR1);
    sigdelset(&mask_set, SIGUSR2); 
    sigaction(SIGUSR1, &sa_usr1, NULL);
    sigaction(SIGUSR2, &sa_usr2, NULL); 

    while(1)
        sigsuspend(&mask_set);

    return -1;
}