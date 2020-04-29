CC = gcc -Wall

all: main.o receiver.o sender.o packer.o
	$(CC) main.o -o main
	$(CC) sender.o -o sender
	$(CC) receiver.o -o receiver
	$(CC) packer.o -o packer

test:
	./main

main.o: main.c utils.h
	$(CC) main.c -c

sender.o: sender.c utils.h
	$(CC) sender.c -c

receiver.o: receiver.c utils.h
	$(CC) receiver.c -c

packer.o: packer.c utils.h
	$(CC) packer.c -c

clean:
	rm *.o main sender packer receiver