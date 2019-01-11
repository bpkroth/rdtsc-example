CC=gcc
CFLAGS=-O -Wall

.PHONY: all
all: rdtsc multicore-readtsc

#rdtsc: a.o b.o
#	@${CC} ${CFLAGS} *.o -o rdtsc

rdtsc: rdtsc.c
	@${CC} ${CFLAGS} rdtsc.c -o rdtsc

multicore-readtsc: multicore-readtsc.c
	@${CC} ${CFLAGS} -lpthread multicore-readtsc.c -o multicore-readtsc

.PHONY: clean
clean:
	@rm -f *.o
	@rm -f rdtsc multicore-readtsc
