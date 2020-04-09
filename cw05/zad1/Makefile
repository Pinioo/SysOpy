CC = gcc -Wall

all: pipe_interpreter.o
	$(CC) pipe_interpreter.o -o pipe_interpreter

pipe_interpreter.o: pipe_interpreter.c
	$(CC) pipe_interpreter.c -c

test: all
	./pipe_interpreter commands.txt

clean:
	rm *.o pipe_interpreter
