#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ftw.h>

int runLs(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if(typeflag == FTW_D){
        pid_t child = fork();
        if(child == 0) {
            pid_t pid = getpid();
            printf("%s PID: %d\n", fpath, pid);
            system("ls -l");
            printf("\n");
            exit(0);
        }
        else {
            wait(NULL);
        }
    }
    return 0;
}

int main(int argc, char** argv){
    if (argc <= 1) {
        printf("No directory specified\n");
    }
    else {
        if(nftw(argv[1], runLs, 20, FTW_PHYS) != 0){
            printf("Something went wrong\n");
        }
    }
    return 0;
}