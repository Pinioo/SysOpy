#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char** argv){
    if(argc < 5){
        puts("Too few arguments\nFormat: ./tester output_file consumer_N number_of_inputs input_file N [input_file N ...]");
        exit(-1);
    }
    char* pipe = "test_pipe";
    mkfifo(pipe, S_IRWXU);

    char* output_file = argv[1];
    char* consumer_n = argv[2];
    int producers = atoi(argv[3]);
    if(argc - 4 < producers*2){
        puts("Too few producers arguments");
        exit(-2);
    }

    // Fork consumer
    if(!fork())
        execl("./consumer", "consumer", pipe, output_file, consumer_n, NULL);


    char* producer_n;
    char* input_file;
    int offset = 4;
    for(int i = 0; i < producers; ++i){
        input_file = argv[offset];
        producer_n = argv[offset+1];
        offset += 2;

        // Fork i-th producer
        if(!fork()){
            execl("./producer", "producer", pipe, input_file, producer_n, NULL);
        }
    }
    while (wait(NULL) > 0);
    remove(pipe);
    return 0;
}