CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99
TARGET  = beeper
SRC     = beeper.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
