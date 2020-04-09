CC = gcc
FL = -Wall

all: myfind.o myfind_nftw.o
	$(CC) $(FL) myfind.o -o myfind
	$(CC) $(FL) myfind_nftw.o -o myfind_nftw

myfind.o: myfind.c
	$(CC) $(FL) -c myfind.c

myfind_nftw.o: myfind_nftw.c
	$(CC) $(FL) -c myfind_nftw.c

clean:
	rm *.o myfind myfind_nftw
