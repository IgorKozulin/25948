CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99
TARGET  = pipe_upper
SRC     = pipe_upper.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
