CC=gcc
CFLAGS=-Wall -pedantic -O3 -ggdb

.PHONY: all clean

all: main.o
	$(CC) $(CFLAGS) main.o -o program

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -rf *.o program