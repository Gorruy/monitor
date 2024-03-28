CC = gcc

CFLAGS = -O2 -Wall

SNFRLDFLAGS = -lpthread -lrt
RPSTLDFLAGS = -lrt

SNFRSRCDIR = src/sniffer/
RPSTSRCDIR = src/representer/
BINDIR = bin/
BUILDDIR = build/
INCLDIR = include/

SNFRSRCS = $(wildcard $(SNFRSRCDIR)*.c)
RPSTSRCS = $(wildcard $(RPSTSRCDIR)*.c)

SNFROBJS := $(SNFRSRCS:.c=.o)
RPSTOBJS := $(RPSTSRCS:.c=.o)

.PHONY: clean trueclean all

all: sniffer representer

sniffer: $(SNFROBJS)
	$(CC) $(CFLAGS) $(SNFRSRCS) -o $(BINDIR)$@ $(SNFRLDFLAGS)

representer: $(RPSTOBJS)
	$(CC) $(CFLAGS) $(RPSTSRCS) -o $(BINDIR)$@ $(RPSTLDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	find . -name \*.o -type f -delete

trueclean: clean
	rm $(BINDIR)/*
