CC = gcc -Wall

all: file_sort.o
	$(CC) file_sort.o -o file_sort

file_sort.o: file_sort.c
	$(CC) file_sort.c -c

test: all
	./file_sort test.txt

clean:
	rm *.o file_sort
