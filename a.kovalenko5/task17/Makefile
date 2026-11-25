CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = line_editor

all: $(TARGET)

$(TARGET): line_editor.c
	$(CC) $(CFLAGS) line_editor.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
