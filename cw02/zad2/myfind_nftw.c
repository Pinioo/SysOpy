#define _XOPEN_SOURCE 500
#define _GNU_SOURCE 500

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <features.h>
#include <ftw.h>

int minMtime = -1;
int maxMtime = INT_MAX;

int minAtime = -1;
int maxAtime = INT_MAX;

int maxDepth = INT_MAX;
char* rp;

char* mtime_str;
char* atime_str;

int showInfo(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if(ftwbuf->level > maxDepth)
        return FTW_SKIP_SIBLINGS;
    char* type;
    switch (typeflag)
    {
    case FTW_D:
    case FTW_DNR:
        type = "dir      ";
        rp = realpath(fpath, NULL);
        break;
    case FTW_F:
        if(S_ISREG(sb->st_mode)){
            type = "file     ";
        }
        else if(S_ISCHR(sb->st_mode)){
            type = "char dev ";
        }
        else if(S_ISBLK(sb->st_mode)){
            type = "block dev";
        }
        else if(S_ISFIFO(sb->st_mode)){
            type = "fifo     ";
        }
        else if(S_ISSOCK(sb->st_mode)){
            type = "sock     ";
        }
        rp = realpath(fpath, NULL);
        break;
    case FTW_SL:
        type = "slink    ";
        char* rp_tmp = realpath(fpath, NULL);
        rp = (char*)malloc(200*sizeof(char));
        sprintf(rp, "%s -> %s", fpath, rp_tmp);
        free(rp_tmp);
        break;
    case FTW_NS:
        rp = realpath(fpath, NULL);
        printf("%s: Permission denied\n", rp);
        free(rp);
        return 0;
    }
    struct tm* tm_mtime = localtime(&sb->st_mtime);
    struct tm* tm_atime = localtime(&sb->st_atime);
    strftime(mtime_str, 18, "%Y %b %d %R", tm_mtime);
    strftime(atime_str, 18, "%Y %b %d %R", tm_atime);
    printf("%s %ld\t%ld\t%s\t%s\t%s\n", type, sb->st_nlink, sb->st_size, mtime_str, atime_str, rp);
    free(rp);
    return 0;
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("You must specify destination directory.\n");
        exit(-1);
    }

    mtime_str = (char*)malloc(18*sizeof(char));
    atime_str = (char*)malloc(18*sizeof(char));

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

    printf("dev type  links\tsize\tmtime\t\t\tatime\t\t\tfull path\n");
    nftw(argv[1], showInfo, 20, FTW_PHYS | FTW_ACTIONRETVAL);
    free(atime_str);    
    free(mtime_str);
    return 0;
}
