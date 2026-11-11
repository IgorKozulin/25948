CC = gcc
CFLAGS = -O2
TARGET = pipe_upper

all: $(TARGET)

$(TARGET): pipe_upper.c
	$(CC) $(CFLAGS) -o $(TARGET) pipe_upper.c

clean:
	rm -f $(TARGET)

.PHONY: all clean

