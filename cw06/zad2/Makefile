CC = gcc -Wall

all: chat_client.o chat_server.o
	$(CC) chat_client.o -o chat_client -lrt
	$(CC) chat_server.o -o chat_server -lrt

chat_server.o: chat_server.c chat_flags.h
	$(CC) chat_server.c -c

chat_client.o: chat_client.c chat_flags.h
	$(CC) chat_client.c -c

clean:
	rm *.o chat_client chat_server
