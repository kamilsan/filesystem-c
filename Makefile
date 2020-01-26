CC=gcc
CFLAGS=-Wall -pedantic -O3 -ggdb

.PHONY: all clean

all: main.o filesystem.o heap.o segment_array.o
	$(CC) $(CFLAGS) main.o filesystem.o heap.o segment_array.o -o program

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

filesystem.o: filesystem.c filesystem.h
	$(CC) $(CFLAGS) -c filesystem.c

heap.o: heap.c heap.h
	$(CC) $(CFLAGS) -c heap.c

segment_array.o: segment_array.c segment_array.h
	$(CC) $(CFLAGS) -c segment_array.c

clean:
	rm -rf *.o program