CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lsocket -lnsl

TARGETS = server client

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

clean:
	rm -f $(TARGETS) socket

.PHONY: all clean
