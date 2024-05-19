###############################################################################
# Makefile for Project, Parallel and Distributed Computing.
###############################################################################

CC = mpicc
CFLAGS = -std=c99 -g -O3
LIBS = -lm

BIN = shearsort

all: $(BIN)

ss: ss.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

shearsort: shearsort.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	$(RM) $(BIN)
