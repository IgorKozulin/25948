CC = gcc
CFLAGS = -O2
TARGET = pipe_caps
SRC = pipe_caps.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean

