#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if(argc < 2){
        printf("File with commands must be specified!\n");
        exit(-1);
    }
    char* line;
    size_t s;
    FILE* file = fopen(argv[1], "r");
    getline(&line, &s, file);
    fclose(file);

    line[strlen(line) - 1] = '\0';

    char* arguments[1000];
    char* argument = strtok(line, " ");
    int childNumber = 0;
    int newPipe[2];
    int oldPipeIn;
    while(argument != NULL){
        char* command = argument;
        arguments[0] = argument;
        argument = strtok(NULL, " ");
        int arg = 1;
        while(argument != NULL && strcmp(argument, "|") != 0){
            if(argument[0] == '\''){
                int index = 1;
                int len = strlen(argument);
                while(index < len + 1){
                    if(argument[index] == '\0'){
                        argument[index] = ' ';
                        break;
                    }
                }
                argument = strtok(&argument[1], "\'");
            }
            else{
                arguments[arg++] = argument;
                argument = strtok(NULL, " ");
            }
        }
        arguments[arg] = NULL;


        if(childNumber > 0){
            oldPipeIn = newPipe[0];
        }
        pipe(newPipe);

        if(fork() == 0){
            close(newPipe[0]);
            if(childNumber > 0)
                dup2(oldPipeIn, STDIN_FILENO);
            dup2(newPipe[1], STDOUT_FILENO);
            execvp(command, arguments);
        }
        else{
            close(newPipe[1]);
        }
        argument = strtok(NULL, " ");
        childNumber++;
    }
    while (wait(NULL) > 0);
    dup2(newPipe[0], STDIN_FILENO);
    execlp("cat", "cat", NULL);
    return 0;
}