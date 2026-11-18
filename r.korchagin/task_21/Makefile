CC = gcc
CFLAGS = -O2
TARGET = signal_beeper
SRC = signal_beeper.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean

