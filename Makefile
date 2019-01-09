CC=gcc
CFLAGS=-O -Wall

.PHONY: all
all: clean rdtsc

#rdtsc: a.o b.o
#	@${CC} ${CFLAGS} *.o -o rdtsc

rdtsc: rdtsc.c
	@${CC} ${CFLAGS} rdtsc.c -o rdtsc

.PHONY: clean
clean:
	@rm -f *.o
	@rm -f rdtsc
