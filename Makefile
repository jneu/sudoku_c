TARGET = sudoku

OBJECTS = sudoku.o

CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -Werror -O2
LDFLAGS =

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

.PHONY: clean
clean:
	rm -rf $(OBJECTS) $(TARGET)
