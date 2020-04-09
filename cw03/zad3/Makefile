CC = gcc -Wall

all: matrix_generator.c multiply_limits.c
	$(CC) matrix_generator.c -o matrix_generator
	$(CC) multiply_limits.c -o multiply_limits

test: all
	./matrix_generator 15 10 20
	./multiply_limits lista 8

clean:
	rm A_* B_* C_* *.o matrix_generator multiply_limits