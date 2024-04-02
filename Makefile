CC = gcc

CFLAGS = -O2 -Wall

SNFRLDFLAGS = -lpthread -lrt
RPSTLDFLAGS = -lrt

SRCDIR = src/
RPSTSRCDIR = src/representer/
BINDIR = bin/
BUILDDIR = build/
INCLDIR = include/

SNFRSRCS = $(wildcard $(SRCDIR)sniffer/*.c)
RPSTSRCS = $(wildcard $(SRCDIR)representer/*.c)

SNFROBJS := $(SNFRSRCS:.c=.o)
RPSTOBJS := $(RPSTSRCS:.c=.o)

.PHONY: clean trueclean all

all: $(BINDIR) sniffer representer

sniffer: $(SNFROBJS)
	$(CC) $(CFLAGS) $^ $(SNFRLDFLAGS) -o $(BINDIR)$@

representer: $(RPSTOBJS)
	$(CC) $(CFLAGS) $^ $(RPSTLDFLAGS) -o $(BINDIR)$@

%.o: %.c
	$(CC) $(CFLAGS) -I./$(INCLDIR) -c $< -o $@

$(BINDIR):
	@mkdir -p $(BINDIR)

clean:
	find . -name \*.o -type f -delete

trueclean: clean
	rm $(BINDIR)/*
