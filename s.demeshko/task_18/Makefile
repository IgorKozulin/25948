CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99
TARGET  = my_ls
SRC     = my_ls.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
