CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE

all: options

options: options.c
	$(CC) $(CFLAGS) -o options options.c

clean:
	rm -f options
