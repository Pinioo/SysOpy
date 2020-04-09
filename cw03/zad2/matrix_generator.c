#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void generateMatrix(int rows, int columns, char* filename){
    FILE* file = fopen(filename, "w");
    srand(time(NULL));
    char* writer = (char*)malloc(5*sizeof(char));
    for(int r = 0; r < rows; r++){
        for(int c = 0; c < columns - 1; c++){
            sprintf(writer, "%d\t", rand() % 201 - 100);
            fwrite(writer, strlen(writer), 1, file);
        }
        sprintf(writer, "%d\n", rand() % 201 - 100);
        fwrite(writer, strlen(writer), 1, file);
    }
    fclose(file);
    free(writer);
}

void generateMatricesToMultiply(int count, int min, int max){
    char* filename_A = (char*)malloc(20*sizeof(char));
    char* filename_B = (char*)malloc(20*sizeof(char));
    for(int i = 1; i <= count; ++i){
        sprintf(filename_A, "A_%d", i);
        sprintf(filename_B, "B_%d", i);

        int rows_A, rows_B;
        int cols_A, cols_B;

        cols_A = rows_B = min + rand() % (max - min + 1);
        rows_A = min + rand() % (max - min + 1);
        cols_B = min + rand() % (max - min + 1);

        generateMatrix(rows_A, cols_A, filename_A);
        generateMatrix(rows_B, cols_B, filename_B);
    }
    free(filename_A);
    free(filename_B);
}

int main(int argc, char** argv){
    if (argc < 4)
        printf("Too few arguments\n");
    else
        generateMatricesToMultiply(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    return 0;
}