CC = gcc -Wall

all: signaled_ls.o
	$(CC) signaled_ls.o -o signaled_ls

signaled_ls.o: signaled_ls.c
	$(CC) signaled_ls.c -c

test: all
	./signaled_ls

clean:
	rm *.o signaled_ls
