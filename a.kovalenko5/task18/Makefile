CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = my_ls

all: $(TARGET)

$(TARGET): my_ls.c
	$(CC) $(CFLAGS) my_ls.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
