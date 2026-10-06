CC = gcc
CFLAGS = -O2
TARGET = list_strings
SRC = list_strings.c

all: $(TARGET)

$(TARGET): $(SRC)
list_strings.c $(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
list_strings.c rm -f $(TARGET)

.PHONY: all clean
