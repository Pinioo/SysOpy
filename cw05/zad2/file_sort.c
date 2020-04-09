#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/unistd.h>

int main(int argc, char** argv){
    if(argc < 1){
        printf("You must specify file name!\n");
        exit(-1);
    }
    char* filename = argv[1];

    int fd[2];

    pipe(fd);
    int pipe_in = fd[1];
    int pipe_out = fd[0];
    if(!fork()){
        // Child to send file content to pipe
        close(pipe_out);

        dup2(pipe_in, STDOUT_FILENO);
        execlp("cat", "cat", filename, NULL);
    }
    else if(!fork()){
        // Child to sort from pipe
        close(pipe_in);

        dup2(pipe_out, STDIN_FILENO);
        execlp("sort", "sort", NULL);
    }
    close(pipe_in);
    close(pipe_out);
    while (wait(NULL) > 0);
    return 0;
}