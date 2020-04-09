#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

typedef struct Dirstack{
    struct Dirstack* next;
    char* dirPath;
    int depth;
} Dirstack;

void put(Dirstack** st, char* path, int depth) {
    Dirstack* tmp = (Dirstack*)malloc(sizeof(Dirstack));
    tmp->depth = depth;
    tmp->dirPath = (char*)malloc((strlen(path) + 1) * sizeof(char));
    strcpy(tmp->dirPath, path);
    tmp->next = *st;
    *st = tmp;
}

Dirstack* pop(Dirstack** st) {
    Dirstack* dir = *st;
    *st = (*st)->next;
    return dir;
}

int main(int argc, char** argv){
    int minMtime = -1;
    int maxMtime = INT_MAX;

    int minAtime = -1;
    int maxAtime = INT_MAX;

    int maxDepth = INT_MAX;

    for(int i = 2; i < argc; i += 2){
        if(strcmp(argv[i], "-mtime") == 0){
            if(i+1 == argc){
                printf("Argument for option -mtime not specified\n");
                return -1;
            }
            switch (argv[i+1][0]){
                case '-': maxMtime = atoi(&argv[i+1][1]); break;
                case '+': minMtime = atoi(&argv[i+1][1]); break;
                default: {
                    minMtime = atoi(argv[i+1]) - 1;
                    maxMtime = minMtime + 2; 
                    break;
                }
            }
        }
        else if(strcmp(argv[i], "-atime") == 0){
            if(i+1 == argc){
                printf("Argument for option -atime not specified\n");
                return -1;
            }
            switch (argv[i+1][0]){
                case '-': maxAtime = atoi(&argv[i+1][1]); break;
                case '+': minAtime = atoi(&argv[i+1][1]); break;
                default: {
                    minAtime = atoi(argv[i+1]) - 1;
                    maxAtime = minAtime + 2; 
                    break;
                }
            }
        }
        else if(strcmp(argv[i], "-maxdepth") == 0){
            if(i+1 == argc){
                printf("Argument for option -maxdepth not specified\n");
                return -1;
            }
            maxDepth = atoi(argv[i+1]);
        }
        else {
            printf("Option %s not available\n", argv[i]);
            return -1;
        }
    }

    Dirstack* stack = NULL;

    put(&stack, argv[1], 1);

    struct dirent* file;
    struct stat fileStat;
    char* mtime_str = (char*)malloc(18*sizeof(char));
    char* atime_str = (char*)malloc(18*sizeof(char));
    char* type;
    printf("dev type  links\tsize\tmtime\t\t\tatime\t\t\tfull path\n");
    while(stack != NULL){
        Dirstack* node = pop(&stack);
        char* path = node->dirPath;
        int depth = node->depth;
        if(depth <= maxDepth){
            DIR* dir = opendir(path);
            if(dir != NULL){
                chdir(path);
                file = readdir(dir);
                while(file != NULL){
                    char* rp = realpath(file->d_name, NULL); 
                    lstat(file->d_name, &fileStat);
                    errno = 0;
                    free(rp);
                    if(errno == 0){
                        int mtime = difftime(time(NULL), fileStat.st_mtime)/(3600*24);
                        int atime = difftime(time(NULL), fileStat.st_atime)/(3600*24);
                        if(strcmp(file->d_name, "..") != 0 && mtime > minMtime && mtime < maxMtime && atime > minAtime && atime < maxAtime){
                            rp = NULL;
                            if(S_ISDIR(fileStat.st_mode)){
                                type = "dir      ";
                                rp = realpath(file->d_name, NULL);
                                if(strcmp(file->d_name, ".") != 0)
                                    put(&stack, rp, depth + 1);
                            }
                            else if(S_ISREG(fileStat.st_mode)){
                                type = "file     ";
                            }
                            else if(S_ISCHR(fileStat.st_mode)){
                                type = "char dev ";
                            }
                            else if(S_ISBLK(fileStat.st_mode)){
                                type = "block dev";
                            }
                            else if(S_ISFIFO(fileStat.st_mode)){
                                type = "fifo     ";
                            }
                            else if(S_ISLNK(fileStat.st_mode)){
                                type = "slink    ";
                                rp = (char*)malloc(300*sizeof(char));
                                char* rp_tmp = realpath(file->d_name, NULL);
                                sprintf(rp, "%s%s -> %s", path, file->d_name, rp_tmp);
                                free(rp_tmp);
                            }
                            else if(S_ISSOCK(fileStat.st_mode)){
                                type = "sock     ";
                            }
                            struct tm* tm_mtime = localtime(&fileStat.st_mtime);
                            struct tm* tm_atime = localtime(&fileStat.st_atime);
                            strftime(mtime_str, 18, "%Y %b %d %R", tm_mtime);
                            strftime(atime_str, 18, "%Y %b %d %R", tm_atime);
                            if(rp == NULL)
                                rp = realpath(file->d_name, NULL);
                            printf("%s %ld\t%ld\t%s\t%s\t%s\n", type, fileStat.st_nlink, fileStat.st_size, mtime_str, atime_str, rp);
                            free(rp);
                        }
                    }
                    errno = 0;
                    file = readdir(dir);
                }
                closedir(dir);
            }
        }
        free(node);
        free(path);
    }
    free(mtime_str);
    free(atime_str);
    return 0;
}