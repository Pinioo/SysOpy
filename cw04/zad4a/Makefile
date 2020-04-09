CC = gcc -Wall

all: catcher.o sender.o
	$(CC) catcher.o -o catcher
	$(CC) sender.o -o sender

catcher.o: catcher.c
	$(CC) catcher.c -c

sender.o: sender.c
	$(CC) sender.c -c

clean:
	rm *.o sender catcher
