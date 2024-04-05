CC = gcc

CFLAGS = -O2 -Wall

SNFRLDFLAGS = -lpthread -lrt
RPSTLDFLAGS = -lrt

SRCDIR = src/
RPSTSRCDIR = src/representer/
BINDIR = bin/
TESTDIR = tests/
BUILDDIR = build/
INCLDIR = include/

prefix = /usr/local

SNFRSRCS = $(wildcard $(SRCDIR)sniffer/*.c)
RPSTSRCS = $(wildcard $(SRCDIR)representer/*.c)
TESTSRCS = $(wildcard $(TESTDIR)*.c)

SNFROBJS := $(SNFRSRCS:.c=.o)
RPSTOBJS := $(RPSTSRCS:.c=.o)
TESTOBJS := $(TESTSRCS:.c=.o)

all: $(BINDIR) build install

build: sniffer representer

install: build
	install -D $(BINDIR)* \
		$(DESTDIR)$(prefix)/bin/

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

tests: build $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTOBJS) $(SRCDIR)sniffer/arg_parser.o $(SNFRLDFLAGS) -o $(TESTDIR)$@

trueclean: clean
	rm $(BINDIR)*

.PHONY: clean trueclean all install build tests
	