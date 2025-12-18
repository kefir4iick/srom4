CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = gf293_onb_test

all: $(TARGET)

$(TARGET): main.c gf293_onb.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c gf293_onb.c

clean:
	rm -f $(TARGET) *.o

test: $(TARGET)
	./$(TARGET)
