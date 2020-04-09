#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>
#include <fcntl.h>

typedef struct Stack{
    int l, r;
    struct Stack* next;
} Stack;

void swap(int* tab, int a, int b){
    int tmp = tab[a];
    tab[a] = tab[b];
    tab[b] = tmp;
}

void addToStack(Stack** st, int l, int r){
    Stack* new = (Stack*)malloc(sizeof(Stack));
    new->l = l;
    new->r = r;
    new->next = *st;
    *st = new;
}

Stack* popFromStack(Stack** st){
    Stack* toRet = *st;
    *st = (*st)->next;
    return toRet;
}

void generateRandFile(char* fileName, int count, int size){
    if(count <= 0 || size <= 0){
        printf("Invalid arguments for generate\n");
        return;
    }
    char* buffer = (char*)malloc((size+1) * sizeof(char));

    if(access(fileName, F_OK) != -1){
        printf("File %s already exists. Do you want to overwrite it? (y/n) ", fileName);
        char command = 'q';
        while (command != 'y' && command != 'n')
            command = getchar();
        if(command == 'y')
            remove(fileName);
        else
            return;
        
    }

    int file = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    int generator = open("/dev/urandom", O_RDONLY);
    
    srand(time(NULL));

    for(int i = 0; i < count; ++i){
        read(generator, buffer, size);
        write(file, buffer, size);
    }
    printf("Random file generated\n");
    close(file);
    close(generator);

    free(buffer);
}

void copyFile(char* fileSrc, char* fileDest, int count, int size) {
    if(count <= 0 || size <= 0){
        printf("Invalid arguments for copy\n");
        return;
    }

    if(access(fileDest, F_OK) != -1){
        printf("File %s already exists. Do you want to overwrite it? (y/n) ", fileDest);
        char command = 'q';
        while (command != 'y' && command != 'n')
            command = getchar();
        if(command == 'y')
            remove(fileDest);
        else
            return; 
    }

    int src = open(fileSrc, O_RDONLY);

    if(src == -1){
        printf("File %s cannot be open\n", fileSrc);
        return;
    }

    int dest = open(fileDest, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    char* buffer = (char*)malloc((size+1) * sizeof(char));

    for(int i = 0; i < count; ++i){
        read(src, buffer, size);
        write(dest, buffer, size);
    }

    printf("File copied succesfully\n");

    close(src);
    close(dest);

    free(buffer);
}

void printFile(char* fileName, int count, int size) {
    if(count <= 0 || size <= 0){
        printf("Invalid arguments for print\n");
        return;
    }

    int file = open(fileName, O_RDONLY);
    if(file == -1){
        printf("File %s cannot be open\n", fileName);
        return;
    }

    char* buffer = (char*)malloc((size+1) * sizeof(char));

    for(int i = 0; i < count; ++i){
        read(file, buffer, size);
        for(int j = 0; j < size && j < 10; ++j){
            printf("%d ", (int)buffer[j]);
        }
        printf("\n");
    }

    close(file);

    free(buffer);
}

void sortFile(char* fileName, int count, int size){
    if(count <= 0 || size <= 0){
        printf("Invalid arguments for generate\n");
        return;
    }

    int file = open(fileName, O_RDONLY);
    if(file == -1){
        printf("File %s cannot be open\n", fileName);
        return;
    }

    char* line_i = (char*)malloc((size + 1) * sizeof(char));
    char* line_j = (char*)malloc(2 * sizeof(char));
    int* positions = (int*)malloc(count*sizeof(int));
    for(int i = 0; i < count; ++i)
        positions[i] = i;

    Stack* stack = NULL;
    addToStack(&stack, 0, count - 1);
    srand(time(NULL));

    while (stack != NULL) {
        Stack* el = popFromStack(&stack);
        if(el->l < el->r){
            int pivot = el->l + rand() % (el->r - el->l);
            swap(positions, pivot, el->r);
            lseek(file, positions[el->r]*size, SEEK_SET);
            read(file, line_i, size);  
            int i = el->l-1;
            int first_eq = el->r;
            for(int j = el->l; j < first_eq; ++j){
                lseek(file, positions[j]*size, SEEK_SET);
                int off = 0;
                do {
                    read(file, line_j, 1);
                } while (line_i[off++] == line_j[0] && off < size);
                --off;

                if(line_j[0] < line_i[off]){
                    swap(positions, ++i, j);
                }
                else if(line_j[0] == line_i[off]){
                    swap(positions, --first_eq, j--);  
                }
            }
            int first_gt = ++i;
            for(int j = first_eq; j <= el->r; ++j){
                swap(positions, j, first_gt++);
            }
            addToStack(&stack, el->l, i-1);
            addToStack(&stack, first_gt, el->r);
        }
        free(el);
    }

    free(line_i);
    free(line_j);   

    char* tmpLine = (char*)malloc((size+1)*sizeof(char));
    char* tmpName = (char*)malloc(20*sizeof(char));
    srand(time(NULL));
    sprintf(tmpName, ".tmp%d.txt", 10000000 + rand()%9000000);

    int tmpFile = open(tmpName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    for(int i = 0; i < count; ++i){
        lseek(file, positions[i] * size, SEEK_SET);
        read(file, tmpLine, size);
        write(tmpFile, tmpLine, size);
    }
    printf("Sorting done\n");

    close(file);
    close(tmpFile);

    remove(fileName);
    rename(tmpName, fileName);

    free(tmpLine);
    free(tmpName);
    free(positions);
}

void addTimes(char* dest, const char* funcName, clock_t start, clock_t end){
    int file = open(dest, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    char* title = (char*)malloc(100*sizeof(char));
    sprintf(title, "Execution time of %s: %f\n", funcName, (double)(end - start)/sysconf(_SC_CLK_TCK));
    write(file, title, strlen(title));
    
    close(file);

    free(title);
}

int main(int argc, char** argv){
    clock_t start;
    clock_t end;
    if(argc < 2){
        printf("Specify command [generate|copy|sort]\n");
    }
    else if(!strcmp(argv[1], "generate")){
        if(argc == 5){
            generateRandFile(argv[2], atoi(argv[3]), atoi(argv[4]));
        }
        else
            printf("Invlid number of arguments for generate (generate [file_name] [number_of_records] [record_size_in_bytes])\n");
    }
    else if(!strcmp(argv[1], "sort")){
        if(argc == 5){
            sortFile(argv[2], atoi(argv[3]), atoi(argv[4]));
        }
        else if(argc == 6){
            start = times(NULL);
            sortFile(argv[2], atoi(argv[3]), atoi(argv[4]));
            end = times(NULL);
            addTimes(argv[5], "sort (system functions)", start, end);
        }
        else
            printf("Invlid number of arguments for sort (sort [file_name] [number_of_records] [record_size_in_bytes] {file_to_save_times})\n");
    }
    else if(!strcmp(argv[1], "copy")){
        if(argc == 6){
            copyFile(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]));
        }
        else if(argc == 7){
            start = times(NULL);
            copyFile(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]));
            end = times(NULL);
            addTimes(argv[6], "copy (system functions)", start, end);
        }
        else
            printf("Invlid number of arguments for copy (copy [source_file_name] [destination_file_name] [number_of_records] [record_size_in_bytes] {file_to_save_times})\n");
    }
    else if(!strcmp(argv[1], "print")){
        if(argc == 5)
            printFile(argv[2], atoi(argv[3]), atoi(argv[4]));
        else
            printf("Invlid number of arguments for print (print [file_name] [number_of_records] [record_size_in_bytes])\n");
    }
    else{
        printf("%s: Command not found. Possible commands: [generate|copy|sort|print]\n", argv[1]);
    }
    return 0;
}