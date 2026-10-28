CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
TARGET = line_editor
SRC = line_editor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
