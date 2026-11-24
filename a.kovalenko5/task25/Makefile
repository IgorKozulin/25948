
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = pipe_upper

all: $(TARGET)

$(TARGET): pipe_upper.c
	$(CC) $(CFLAGS) pipe_upper.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean

