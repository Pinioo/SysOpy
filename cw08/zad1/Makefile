CC = gcc -Wall

all: main.o utils.o
	$(CC) main.o utils.o -o main -lpthread

main.o: main.c
	$(CC) main.c -c

utils.o: utils.c
	$(CC) utils.c -c

clean:
	rm *.o main hist_*

test: all
	./main sign 1 coins.ascii.pgm
	./main sign 2 coins.ascii.pgm
	./main sign 4 coins.ascii.pgm
	./main sign 8 coins.ascii.pgm
	./main block 1 coins.ascii.pgm
	./main block 2 coins.ascii.pgm
	./main block 4 coins.ascii.pgm
	./main block 8 coins.ascii.pgm
	./main interleaved 1 coins.ascii.pgm
	./main interleaved 2 coins.ascii.pgm
	./main interleaved 4 coins.ascii.pgm
	./main interleaved 8 coins.ascii.pgm

times: all
	echo "----------SIGN----------" > Times.txt
	echo ">>Threads: 1" >> Times.txt
	./main sign 1 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 2" >> Times.txt
	./main sign 2 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 4" >> Times.txt
	./main sign 4 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 8" >> Times.txt
	./main sign 8 coins.ascii.pgm >> Times.txt
	echo "----------BLOCK----------" >> Times.txt
	echo ">>Threads: 1" >> Times.txt
	./main block 1 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 2" >> Times.txt
	./main block 2 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 4" >> Times.txt
	./main block 4 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 8" >> Times.txt
	./main block 8 coins.ascii.pgm >> Times.txt
	echo "----------INTERLEAVED----------" >> Times.txt
	echo ">>Threads: 1" >> Times.txt
	./main interleaved 1 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 2" >> Times.txt
	./main interleaved 2 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 4" >> Times.txt
	./main interleaved 4 coins.ascii.pgm >> Times.txt
	echo ">>Threads: 8" >> Times.txt
	./main interleaved 8 coins.ascii.pgm >> Times.txt