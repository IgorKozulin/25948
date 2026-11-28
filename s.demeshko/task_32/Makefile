CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99
LIBS    = -lrt
TARGETS = server client

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LIBS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LIBS)

clean:
	rm -f $(TARGETS) socket *.o

.PHONY: all clean
