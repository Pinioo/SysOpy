CC = gcc -Wall

all: consumer.o producer.o tester.o
	$(CC) consumer.o -o consumer
	$(CC) producer.o -o producer
	$(CC) tester.o -o tester
	mkfifo mypipe

producer.o: producer.c
	$(CC) producer.c -c

consumer.o: consumer.c
	$(CC) consumer.c -c

tester.o: tester.c
	$(CC) tester.c -c

test: all
	./tester output.txt 10 4 test1.txt 10 test2.txt 15 test3.txt 8 test4.txt 1

clean:
	rm *.o output.txt tester producer consumer mypipe
