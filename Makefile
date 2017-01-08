TARGET = sudoku

OBJECTS = sudoku.o

CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -Werror -O0 -g
LDFLAGS = -g

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

.PHONY: clean
clean:
	rm -rf $(OBJECTS) $(TARGET) $(TARGET).dSYM *~
