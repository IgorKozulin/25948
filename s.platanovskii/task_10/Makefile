CC = gcc
CFLAGS = -O2
TARGET = exec_status
SRC = exec_status.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
