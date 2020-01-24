CC=gcc
CFLAGS=-Wall -pedantic -O3 -ggdb

.PHONY: all clean

all: main.o filesystem.o heap.o
	$(CC) $(CFLAGS) main.o filesystem.o heap.o -o program

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

filesystem.o: filesystem.c filesystem.h
	$(CC) $(CFLAGS) -c filesystem.c

heap.o: heap.c heap.h
	$(CC) $(CFLAGS) -c heap.c

clean:
	rm -rf *.o program