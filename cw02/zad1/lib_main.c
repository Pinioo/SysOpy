#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

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

    FILE* file = fopen(fileName, "a");
    FILE* generator = fopen("/dev/urandom", "r");
    
    srand(time(NULL));

    for(int i = 0; i < count; ++i){
        fread(buffer, size, 1, generator);
        fwrite(buffer, size, 1, file);
    }
    printf("Random file generated\n");
    fclose(file);
    fclose(generator);

    free(buffer);
}

void copyFile(char* fileSrc, char* fileDest, int count, int size) {
    if(count <= 0 || size <= 0){
        printf("Invalid arguments for copy\n");
        return;
    }


    if(access(fileSrc, F_OK) == -1){
        printf("File %s doesn't exist\n", fileSrc);
        exit(-1);
        
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

    char* buffer = (char*)malloc((size+1) * sizeof(char));

    FILE* src = fopen(fileSrc, "r");
    FILE* dest = fopen(fileDest, "a");

    for(int i = 0; i < count; ++i){
        fread(buffer, size, 1, src);
        fwrite(buffer, size, 1, dest);
    }

    printf("File copied succesfully\n");

    fclose(src);
    fclose(dest);

    free(buffer);
}

void printFile(char* fileName, int count, int size) {
    if(count <= 0 || size <= 0){
        printf("Invalid arguments for print\n");
        return;
    }
    else if(access(fileName, F_OK) == -1){
        printf("File %s doesn't exist\n", fileName);
        exit(-1);
        
    }

    char* buffer = (char*)malloc((size+1) * sizeof(char));

    FILE* file = fopen(fileName, "r");

    for(int i = 0; i < count; ++i){
        fread(buffer, size, 1, file);
        for(int j = 0; j < size && j < 10; ++j){
            printf("%d ", (int)buffer[j]);
        }
        printf("\n");
    }

    fclose(file);

    free(buffer);
}

void sortFile(char* fileName, int count, int size){
    if(count <= 0 || size <= 0){
        printf("Invalid arguments for generate\n");
        return;
    }
    
    if(access(fileName, F_OK) == -1){
        printf("File %s doesn't exist\n", fileName);
        exit(-1);
    }

    FILE* file = fopen(fileName, "r");
    
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
            fseek(file, positions[el->r]*size, SEEK_SET);
            fread(line_i, size, 1, file);  
            int i = el->l-1;
            int first_eq = el->r;
            for(int j = el->l; j < first_eq; ++j){
                fseek(file, positions[j]*size, SEEK_SET);
                int off = 0;
                do
                {
                    fread(line_j, 1, 1, file);
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

    FILE* tmpFile = fopen(tmpName, "a");

    for(int i = 0; i < count; ++i){
        fseek(file, positions[i] * size, SEEK_SET);
        fread(tmpLine, size, 1, file);
        fwrite(tmpLine, size, 1, tmpFile);
    }

    printf("Sorting done.\n");

    fclose(file);
    fclose(tmpFile);

    remove(fileName);
    rename(tmpName, fileName);

    free(tmpLine);
    free(tmpName);
    free(positions);
}

void addTimes(char* dest, const char* funcName, clock_t start, clock_t end){
    FILE* file = fopen(dest, "a");
    char* title = (char*)malloc(100*sizeof(char));
    sprintf(title, "Execution time of %s: %f\n", funcName, (double)(end - start)/sysconf(_SC_CLK_TCK));
    fwrite(title, strlen(title), 1, file);
    
    fclose(file);

    free(title);
}

int main(int argc, char** argv){
    clock_t start;
    clock_t end;
    if(argc < 2){
        printf("Specify command [generate|copy|sort]\n");
    }

    else if(!strcmp(argv[1], "generate")){
        if(argc == 5)
            generateRandFile(argv[2], atoi(argv[3]), atoi(argv[4]));
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
            addTimes(argv[5], "sort (library functions)", start, end);
        }
        else
            printf("Invlid number of arguments for sort (sort [file_name] [number_of_records] [record_size_in_bytes] {file_to_save_times}\n");
    }
    else if(!strcmp(argv[1], "copy")){
        if(argc == 6){
            copyFile(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]));
        }
        else if(argc == 7){
            start = times(NULL);
            copyFile(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]));
            end = times(NULL);
            addTimes(argv[6], "copy (library functions)", start, end);
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