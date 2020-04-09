CC = gcc -Wall

all: sig_inherit.o sig_raiser.o
	$(CC) sig_inherit.o -o sig_inherit
	$(CC) sig_raiser.o -o sig_raiser

sig_inherit.o: sig_inherit.c
	$(CC) sig_inherit.c -c

sig_raiser.o: sig_raiser.c
	$(CC) sig_raiser.c -c

test: all
	echo "Handler"
	./sig_inherit handler || echo -n ''
	echo ""
	echo "Ignore"
	./sig_inherit ignore || echo -n ''
	echo ""
	echo "Mask"
	./sig_inherit mask || echo -n ''
	echo ""
	echo "Pending"
	./sig_inherit pending || echo -n ''

clean:
	rm *.o sig_inherit sig_raiser
