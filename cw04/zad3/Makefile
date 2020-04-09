CC = gcc -Wall

all: sig_info.o
	$(CC) sig_info.o -o sig_info

sig_info.o: sig_info.c
	$(CC) sig_info.c -c

clean:
	rm *.o sig_info

test: all
	./sig_info
