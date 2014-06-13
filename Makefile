CFLAGS = -O3 -g -Wall -ansi -pedantic -std=c99

CC = gcc

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

OBJS = ggets.o handle_ferr.o aa-tree.o

all: sort aa-test1

sort: $(OBJS) ll3.o sort.o
	$(CC) $(OBJS) ll3.o sort.o -o sort

aa-test1: $(OBJS) ll3.o aa-test1.o
	$(CC) $(OBJS) aa-test1.o -o aa-test1

clean:
	rm *.o sort aa-test1
