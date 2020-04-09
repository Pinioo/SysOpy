#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/signal.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

extern const char *const sys_siglist[];

void handler(int sig, siginfo_t* info, void *ucontext){
    register struct passwd *pw;
    pw = getpwuid(info->si_uid);

    char code[10];

    switch(info->si_code){
        case SI_KERNEL:
            sprintf(code, "kernel");
            break;
        case SI_USER:
            sprintf(code, "user kill");
            break;
        default:
            sprintf(code, "%d", info->si_code);
            break;
    }

    char file[10];

    switch (info->si_fd)
    {
        case 0:
            sprintf(file, "stdin");
            break;
        case 1:
            sprintf(file, "stdout");
            break;
        case 2:
            sprintf(file, "stderr");
            break;
        default:
            sprintf(file, "%d", info->si_code);
            break;
    }

    printf("\nSIG%s \tSending Process PID: %d \tSending user: %s \tCode: %s \tFile: %s\n", sys_siglist[sig], info->si_pid, pw->pw_name, code, file);
}

int main(){
    sigset_t full_mask;
    sigset_t empty_mask;
    struct sigaction sa = {.sa_flags = SA_SIGINFO, .sa_sigaction = &handler};
    sigfillset(&sa.sa_mask);

    sigemptyset(&empty_mask);
    sigfillset(&full_mask);

    sigprocmask(SIG_BLOCK, &full_mask, NULL);

    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    while(1){
        sigsuspend(&empty_mask);
    }
    return 0;
}
