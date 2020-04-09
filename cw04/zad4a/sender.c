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

void kill_sending(){
    for(int i = 0; i < sigs_to_send; ++i){
        kill(catcher_pid, SIGUSR1);
    } 
    kill(catcher_pid, SIGUSR2);
}

void queue_sending(){
    union sigval sv;
    sv.sival_int = SIGEV_NONE;
    for(int i = 0; i < sigs_to_send; ++i){
        sigqueue(catcher_pid, SIGUSR1, sv);
    } 
    sigqueue(catcher_pid, SIGUSR2, sv);
}

void rt_sending(){
    for(int i = 0; i < sigs_to_send; ++i){
        kill(catcher_pid, SIGRTMIN);
    } 
    kill(catcher_pid, SIGRTMAX);
}

void usr1_reaction(){
    ++usr1_caught;
}

void usr2_reaction(){
    printf("Sender caught %d/%d signals\n", usr1_caught, sigs_to_send);
    exit(0);
}

int main(int argc, char** argv){
    if(argc < 4){
        printf("Too few arguments\n");
        exit(-1);
    }

    catcher_pid = atoi(argv[1]);
    sigs_to_send = atoi(argv[2]);
    char* mode = argv[3];

    struct sigaction sa_usr1;
    struct sigaction sa_usr2;

    sigfillset(&sa_usr1.sa_mask);
    sigfillset(&sa_usr2.sa_mask);

    sa_usr1.sa_flags = 0;
    sa_usr2.sa_flags = 0;

    sa_usr1.sa_handler = &usr1_reaction;
    sa_usr2.sa_handler = &usr2_reaction;

    
    sig_mode send_mode = get_mode(mode);
    
    switch(send_mode) {
        case KILL:
        case QUEUE: 
            sigaction(SIGUSR1, &sa_usr1, NULL);
            sigaction(SIGUSR2, &sa_usr2, NULL);
            break;
        case RT: 
            sigaction(SIGRTMIN, &sa_usr1, NULL);
            sigaction(SIGRTMAX, &sa_usr2, NULL);
            break;
        default: 
            printf("Mode is incorrect\n"); 
            exit(-1);
    }

    switch(send_mode) {
        case KILL: kill_sending(); break;
        case QUEUE: queue_sending(); break;
        case RT: rt_sending(); break;
        default: printf("Mode is incorrect\n"); exit(-1);
    }
    while(1){
        pause();
    }
    return 0;
}