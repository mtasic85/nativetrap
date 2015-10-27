# clear; make clean; make && ls -l vm && time ./vm

# CC=gcc
CC=clang
CFLAGS=-c -Wall -O3
LDFLAGS=
SRCS=vm.c code.c string.c struct.c union.c enum.c
OBJS=$(SRCS:.c=.o)
EXE=vm

all: $(SRCS) $(EXE)

$(EXE): $(OBJS) 
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJS)
	rm $(EXE)
