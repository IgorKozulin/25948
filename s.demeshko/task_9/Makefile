CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99

all: process_manager

process_manager: process_manager.c
	$(CC) $(CFLAGS) -o $@ $<

nowait: process_manager_nowait

process_manager_nowait: process_manager.c
	$(CC) $(CFLAGS) -DNO_WAIT -o $@ $<

clean:
	rm -f process_manager process_manager_nowait

.PHONY: all nowait clean
