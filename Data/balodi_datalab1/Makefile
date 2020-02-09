CC = gcc
CFLAGS = -Og -Wall -m32 -g -fwrapv
LIBS = -lm

all: btest

btest: btest.c bits.c decl.c tests.c btest.h bits.h
	$(CC) $(CFLAGS) $(LIBS) -o btest bits.c btest.c decl.c tests.c

# Forces a recompile. Used by the grading program.
btestexplicit:
	$(CC) $(CFLAGS) $(LIBS) -o btest bits.c btest.c decl.c tests.c

clean:
	rm -f *.o btest *~
