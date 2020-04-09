CC = gcc -Wall

all: matrix_generator.c multiply.c
	$(CC) matrix_generator.c -o matrix_generator
	$(CC) multiply.c -o multiply

test: all
	./matrix_generator 15 10 20
	./multiply lista 8

clean:
	rm A_* B_* C_* *.o matrix_generator multiply
