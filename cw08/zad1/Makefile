CC = gcc -Wall

all: main.o utils.o
	$(CC) main.o utils.o -o main -lpthread

main.o: main.c
	$(CC) main.c -c

utils.o: utils.c
	$(CC) utils.c -c

clean:
	rm *.o main *.txt

test: all
	./main sign 1
	./main sign 2
	./main sign 4
	./main sign 8
	./main block 1
	./main block 2
	./main block 4
	./main block 8
	./main interleaved 1
	./main interleaved 2
	./main interleaved 4
	./main interleaved 8

times: all
	echo "----------SIGN----------" > Times.txt
	echo ">>Threads: 1" >> Times.txt
	./main sign 1 >> Times.txt
	echo ">>Threads: 2" >> Times.txt
	./main sign 2 >> Times.txt
	echo ">>Threads: 4" >> Times.txt
	./main sign 4 >> Times.txt
	echo ">>Threads: 8" >> Times.txt
	./main sign 8 >> Times.txt
	echo "----------BLOCK----------" >> Times.txt
	echo ">>Threads: 1" >> Times.txt
	./main block 1 >> Times.txt
	echo ">>Threads: 2" >> Times.txt
	./main block 2 >> Times.txt
	echo ">>Threads: 4" >> Times.txt
	./main block 4 >> Times.txt
	echo ">>Threads: 8" >> Times.txt
	./main block 8 >> Times.txt
	echo "----------INTERLEAVED----------" >> Times.txt
	echo ">>Threads: 1" >> Times.txt
	./main interleaved 1 >> Times.txt
	echo ">>Threads: 2" >> Times.txt
	./main interleaved 2 >> Times.txt
	echo ">>Threads: 4" >> Times.txt
	./main interleaved 4 >> Times.txt
	echo ">>Threads: 8" >> Times.txt
	./main interleaved 8 >> Times.txt