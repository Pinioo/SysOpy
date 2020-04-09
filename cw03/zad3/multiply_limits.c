#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/time.h>

struct rlimit cpuLimit;
struct rlimit asLimit;

const char* communicationFile = ".nexttodo";
int operations;
char*** filesArr;

char* columnFile(char* outputFile, int columnIndex){
    char* filename = (char*)malloc(100*sizeof(char));
    sprintf(filename, ".%s_col%d", outputFile, columnIndex);
    return filename;
}

typedef struct Matrix{
    int columns, rows;
    int** data;
} Matrix;

Matrix* createMatrix(int rows, int columns){
    Matrix* created = (Matrix*)malloc(sizeof(Matrix));
    created->rows = rows;
    created->columns = columns;

    created->data = (int**)malloc(rows*sizeof(int*));
    for(int i = 0; i < rows; ++i)
        created->data[i] = (int*)malloc(columns*sizeof(int));

    return created;
}

Matrix* readMatrix(char* filename){
    FILE* file = fopen(filename, "r");

    int rows = 0;
    size_t len = 0;

    char* line = NULL;

    while(getline(&line, &len, file) != EOF)
        ++rows;

    rewind(file);

    char c;
    int columns = 1;
    while((c = getc(file)) != '\n'){
        if (c == '\t')
            ++columns;
    }

    rewind(file);

    Matrix* matrix = createMatrix(rows, columns);

    for(int i = 0; i < rows; ++i){
        getline(&line, &len, file);
        char* part = strtok(line, "\t");
        for(int j = 0; j < columns; ++j){
            matrix->data[i][j] = atoi(part);
            part = strtok(NULL, "\t");
        }
    }

    free(line);
    fclose(file);

    return matrix;
}

void showMatrix(Matrix* matrix){
    for(int i = 0; i < matrix->rows; ++i){
        for(int j = 0; j < matrix->columns; ++j)
            printf("%d\t", matrix->data[i][j]);
        printf("\n");
    }
}

void freeMatrix(Matrix* matrix){
    for(int i = 0; i < matrix->rows; ++i)
        free(matrix->data[i]);
    free(matrix->data);
    free(matrix);
}

int rowColumnProduct(Matrix* A, Matrix* B, int row, int column){
    int sum = 0;
    for(int i = 0; i < A->columns; ++i)
        sum += A->data[row][i]*B->data[i][column];

    return sum;
}


int calculateColumns(){
    int operationsDone = 0;

    while(1){
        FILE* commFile = fopen(communicationFile, "rb+");
        flock(fileno(commFile), LOCK_EX);
        char numbers[3];
        fread(numbers, sizeof(numbers), 1, commFile);
        int lineNo = (int)numbers[0];
        int column = (int)numbers[2];

        if(lineNo >= operations){
            flock(fileno(commFile), LOCK_UN);
            fclose(commFile);
            exit(operationsDone);
        }

        fseek(commFile, 0, SEEK_SET);
        fflush(commFile);

        Matrix* A = readMatrix(filesArr[lineNo][0]);
        Matrix* B = readMatrix(filesArr[lineNo][1]);
        if(column + 1 >= B->columns)
            fprintf(commFile, "%c %c", (char)lineNo + 1, (char)0);
        else
            fprintf(commFile, "%c %c", (char)lineNo, (char)column + 1);
        flock(fileno(commFile), LOCK_UN);
        fclose(commFile);

        char* outputName = columnFile(filesArr[lineNo][2], column);
        FILE* outputFile = fopen(outputName, "w");
        free(outputName);

        char* buffer = (char*)malloc(30*sizeof(char));
        for(int row = 0; row < A->rows - 1; ++row){
            sprintf(buffer, "%d\n", rowColumnProduct(A, B, row, column));
            fwrite(buffer, strlen(buffer), 1, outputFile);
        }  
        sprintf(buffer, "%d", rowColumnProduct(A, B, A->rows - 1, column));
        fwrite(buffer, strlen(buffer), 1, outputFile);

        freeMatrix(A);
        freeMatrix(B);
        
        ++operationsDone;
        free(buffer);
        fclose(outputFile);
    }
}

int main(int argc, char** argv){
    char* line = (char*)malloc(1000*sizeof(char));
    FILE* listFile = fopen(argv[1], "r");
    if(argc >= 5){
        cpuLimit.rlim_cur = cpuLimit.rlim_max = atoi(argv[3]);
        asLimit.rlim_cur = asLimit.rlim_max = atoi(argv[4]);
    }

    size_t len = 0;

    operations = 0;
    while(getline(&line, &len, listFile) != EOF){
        ++operations;
    }

    rewind(listFile);

    filesArr = (char***)malloc(operations*sizeof(char**));
    int i = 0;

    while(getline(&line, &len, listFile) != EOF){
        filesArr[i] = (char**)malloc(3*sizeof(char*));
        for(int j = 0; j < 3; ++j)
            filesArr[i][j] = (char*)malloc(50*sizeof(char));
        strcpy(filesArr[i][0], strtok(line, " "));
        strcpy(filesArr[i][1], strtok(NULL, " "));
        strcpy(filesArr[i][2], strtok(NULL, "\n"));
        ++i;
    }
    fclose(listFile);

    FILE* todo = fopen(communicationFile, "wb");
    fprintf(todo, "%c %c", 0, 0);
    fclose(todo);

    int processes = atoi(argv[2]);
    pid_t* procPID = (pid_t*)malloc(processes * sizeof(pid_t));
    for(int pr = 0; pr < processes; ++pr){
        pid_t childPID = fork();
        if(childPID == 0){
            setrlimit(RLIMIT_CPU, &cpuLimit);
            setrlimit(RLIMIT_AS, &asLimit);
            calculateColumns();
        }
        else{
            procPID[pr] = childPID;
        }
    }
    for(int pr = 0; pr < processes; ++pr){
        int childStatus;
        struct rusage usgBefore;
        getrusage(RUSAGE_CHILDREN, &usgBefore);
        waitpid(procPID[pr], &childStatus, 0);
        struct rusage usgAfter;
        getrusage(RUSAGE_CHILDREN, &usgAfter);
        printf("==%d==\tSYS: %lums\t USR: %lums\n", 
            procPID[pr], 
            (usgAfter.ru_stime.tv_usec - usgBefore.ru_stime.tv_usec)/1000, 
            (usgAfter.ru_utime.tv_usec - usgBefore.ru_utime.tv_usec)/1000
        );
        if(WIFEXITED(childStatus))
            printf("Proces %d wykonal %d mnozen macierzy\n", procPID[pr], WEXITSTATUS(childStatus));
    }
    remove(communicationFile);
    free(procPID);

    for(int lineNo = 0; lineNo < operations; ++lineNo){
        Matrix* B = readMatrix(filesArr[lineNo][1]);
        int cols = B->columns;
        freeMatrix(B);

        char** pasteArguments = (char**)malloc((cols + 2)*sizeof(char*));
        pasteArguments[0] = "paste";
        pasteArguments[cols + 1] = NULL;
        
        for(int j = 0; j < cols; ++j){
            pasteArguments[j + 1] = columnFile(filesArr[lineNo][2], j);
        }
            
        FILE* outputFile = fopen(filesArr[lineNo][2], "w");
        dup2(fileno(outputFile), 1);
        fclose(outputFile);

        pid_t childPID = fork();
        if(childPID == 0){
            execvp("paste", pasteArguments);
            exit(0);
        }
        wait(NULL);

        for(int j = 1; j <= cols; ++j){
            remove(pasteArguments[j]);
            free(pasteArguments[j]);
        }
        free(pasteArguments);
    }
    wait(NULL);
    return 0;
}