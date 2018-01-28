CC=gcc
CFLAGS=-c -Wall -O0 -g
LDFLAGS=-lpthread
SOURCES=main.c ctx.c obj.c bool.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=obj

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS)
	rm $(EXECUTABLE)