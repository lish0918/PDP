###############################################################################
# Makefile for Project, Parallel and Distributed Computing.
###############################################################################

CC = mpicc
CFLAGS = -std=c99 -g
LIBS = -lm

BIN = shearsort

all: $(BIN)

shearsort: shearsort.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	
clean:
	$(RM) $(BIN)
