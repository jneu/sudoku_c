CC := gcc
LD := gcc

CFLAGS := -Wall -Wextra -Werror
CFLAGS += -pedantic -ansi
CFLAGS += -D_GNU_SOURCE
CFLAGS += -MMD
CFLAGS += -I/opt/local/include
CFLAGS += -O0 -g

LDFLAGS := -L/opt/local/lib
LDFLAGS += -lreadline
LDFLAGS += -g

TARGET := sudoku

SOURCES := \
	grid.c \
	sudoku.c

HEADERS := \
	grid.h

OBJECTS := $(SOURCES:.c=.o)
DEPS := $(SOURCES:.c=.d)

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

.PHONY: clean
clean:
	rm -rf $(OBJECTS) $(DEPS) $(TARGET) $(TARGET).dSYM *~

.PHONY: indent
indent:
	for f in $(SOURCES) $(HEADERS); do gindent -gnu -l120 -nut $$f; done

-include $(DEPS)
