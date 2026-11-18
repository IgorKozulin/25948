CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99
TARGET  = run_command
SRC     = run_command.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) crash

.PHONY: all clean
