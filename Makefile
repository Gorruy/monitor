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

all: build tests

build: $(BINDIR) sniffer representer

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

install: build
	install -D $(BINDIR)sniffer \
		$(DESTDIR)$(prefix)/bin/sniffer & \
	install -D $(BINDIR)representer \
		$(DESTDIR)$(prefix)/bin/representer

tests: CFLAGS += -DDEBUG -g
tests: build $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTOBJS) $(SRCDIR)sniffer/arg_parser.o $(SNFRLDFLAGS) -o $(TESTDIR)$@

trueclean: clean
	find ${TESTDIR} -type f -not \( -name '*.c' -or -name '*.sh' \) -delete;
	rm $(BINDIR)*

.PHONY: clean trueclean all install build tests
	