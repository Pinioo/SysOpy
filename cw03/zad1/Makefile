CC = gcc -Wall

all: process_ls.c
	$(CC) process_ls.c -o process_ls

test: all
	./process_ls .

clean:
	rm process_ls
