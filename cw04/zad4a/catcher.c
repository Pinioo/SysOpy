#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <string.h>
#include <sys/types.h>

int usr1_caught = 0;
int mode;

typedef enum sig_mode {
    KILL, QUEUE, RT, UNDEFINED
} sig_mode;

sig_mode get_mode(char* name){
    if(!strcmp(name, "kill"))
        return KILL;
    if(!strcmp(name, "sigqueue"))
        return QUEUE;
    if(!strcmp(name, "sigrt"))
        return RT;
    return UNDEFINED;
}

void usr1_reaction(){
    ++usr1_caught;
}

void usr2_reaction_kill(int sig, siginfo_t* info, void *ucontext){
    for(int i = 0; i < usr1_caught; ++i){
        kill(info->si_pid, SIGUSR1);
    }
    kill(info->si_pid, sig);
    exit(0);
}

void usr2_reaction_queue(int sig, siginfo_t* info, void *ucontext){
    union sigval sv;
    sv.sival_int = SIGEV_NONE;
    for(int i = 0; i < usr1_caught; ++i){
        sigqueue(info->si_pid, SIGUSR1, sv);
    }
    sigqueue(info->si_pid, SIGUSR2, sv);
    exit(0);
}

void usr2_reaction_rt(int sig, siginfo_t* info, void *ucontext){
    for(int i = 0; i < usr1_caught; ++i){
        kill(info->si_pid, SIGRTMIN);
    }
    kill(info->si_pid, SIGRTMAX);
    exit(0);
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("Too few arguments\n");
        exit(-1);
    }
    printf("PID: %d\n", getpid());

    struct sigaction sa_usr1;
    struct sigaction sa_usr2;

    mode = get_mode(argv[1]);

    sa_usr1.sa_handler = &usr1_reaction;
    switch (mode){
        case KILL: sa_usr2.sa_sigaction = &usr2_reaction_kill; break;
        case QUEUE: sa_usr2.sa_sigaction = &usr2_reaction_queue; break;
        case RT: sa_usr2.sa_sigaction = &usr2_reaction_rt; break;
        default:
            printf("Incorrect mode\n");
            exit(-1);
    }

    sa_usr1.sa_flags = 0;
    sa_usr2.sa_flags = SA_SIGINFO;

    sigset_t mask_set;
    sigfillset(&mask_set);

    switch (mode){
        case KILL:
        case QUEUE:
            sigdelset(&mask_set, SIGUSR1);
            sigdelset(&mask_set, SIGUSR2); 
            sigaction(SIGUSR1, &sa_usr1, NULL);
            sigaction(SIGUSR2, &sa_usr2, NULL); 
            break;
        case RT:  
            sigdelset(&mask_set, SIGRTMIN);
            sigdelset(&mask_set, SIGRTMAX);
            sigaction(SIGRTMIN, &sa_usr1, NULL);
            sigaction(SIGRTMAX, &sa_usr2, NULL); 
            break;
        default:
            printf("Incorrect mode\n");
            exit(-1);
    }

    sigprocmask(SIG_SETMASK, &mask_set, NULL);

    while(1)
        pause();
    return -1;
}