
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET = beeper

all: $(TARGET)

$(TARGET): beeper.c
	$(CC) $(CFLAGS) beeper.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean

