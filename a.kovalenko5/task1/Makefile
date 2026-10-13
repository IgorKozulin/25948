CC = gcc
CFLAGS = -O2 -Wall
TARGET = options
SRC = options.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) $(ARGS)

rebuild: clean all

clean:
	rm -f $(TARGET)

.PHONY: all run rebuild clean
