CC = gcc -Wall

all: main.o
	$(CC) main.o -o main -lpthread

main.o: main.c
	$(CC) main.c -c

clean:
	rm *.o main

test: all
	./main 1 8

