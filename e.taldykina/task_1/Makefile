CC = gcc
CFLAGS = -Wall -g
TARGET = task1

all: $(TARGET)

$(TARGET): task1.c
	$(CC) $(CFLAGS) task1.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
