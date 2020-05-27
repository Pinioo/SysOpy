CC = gcc -Wall

all: server.o client.o
	$(CC) server.o -o server -lpthread
	$(CC) client.o -o client -lpthread

server.o: server.c utils.h
	$(CC) server.c -c

client.o: client.c utils.h
	$(CC) client.c -c

clean:
	rm *.o server client