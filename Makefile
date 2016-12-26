CFLAGS = -O3 -g -Wall -ansi -pedantic -std=c99 -march=native

CC = gcc

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

OBJS = ggets.o handle_ferr.o aa-tree.o

SRCS = ggets.c handle_ferr.c aa-tree.c

all: sort2 sort aa-test1 nuniq

wp: sort2-wp sort-wp aa-test1-wp

sort2: $(OBJS) asprintf.o ll3.o sort2.o
	$(CC) $(OBJS) asprintf.o ll3.o sort2.o -o sort2

nuniq: $(OBJS) nuniq.o
	$(CC) $(OBJS) nuniq.o -o nuniq

sort2-wp: $(SRCS) asprintf.c ll3.c sort2.c
	$(CC) $(SRCS) $(CFLAGS) asprintf.c ll3.c sort2.c -o sort2-wp -fwhole-program -flto

sort: $(OBJS) ll3.o sort.o
	$(CC) $(OBJS) ll3.o sort.o -o sort

sort-wp: $(SRCS) ll3.c sort.c
	$(CC) $(SRCS) $(CFLAGS) ll3.c sort.c -o sort-wp -fwhole-program -flto

aa-test1: $(OBJS) ll3.o aa-test1.o
	$(CC) $(OBJS) aa-test1.o -o aa-test1

aa-test1-wp: $(SRCS) ll3.c aa-test1.c
	$(CC) $(SRCS) $(CFLAGS) ll3.c aa-test1.c -o aa-test1-wp -fwhole-program -flto

clean:
	rm *.o sort aa-test1 sort2 sort-wp aa-test1-wp sort2-wp nuniq
