CC = gcc

CFLAGS = -O2 -Wall -Wextra -D_POSIX_C_SOURCE=200112L
LDFLAGS = -lsocket -lnsl
TARGETS = uds_server uds_client

all: $(TARGETS)

uds_server: uds_server.c
	$(CC) $(CFLAGS) -o uds_server uds_server.c $(LDFLAGS)

uds_client: uds_client.c
	$(CC) $(CFLAGS) -o uds_client uds_client.c $(LDFLAGS)

clean:
	rm -f $(TARGETS) *.o socket

.PHONY: all clean

