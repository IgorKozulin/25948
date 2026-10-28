CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O2
TARGET = run_command
SRC = run_command.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
