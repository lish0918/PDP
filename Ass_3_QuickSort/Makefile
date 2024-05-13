###############################################################################
# Makefile for assignment 2, Parallel and Distributed Computing.
###############################################################################

CC = mpicc
CFLAGS = -std=c99 -g -O3
LIBS = -lm

BIN = quicksort

all: $(BIN)

qs: qs.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

quicksort: quicksort.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

reverse: reverse.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	$(RM) $(BIN)
