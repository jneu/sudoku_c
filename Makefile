CC := gcc
LD := gcc

CFLAGS := -Wall -Wextra -Werror
CFLAGS += -pedantic -ansi
CFLAGS += -D_GNU_SOURCE
CFLAGS += -MMD
CFLAGS += -O0 -g

CFLAGS_BATCH :=

CFLAGS_INTERACTIVE := -I/opt/local/include

LDFLAGS := -g

LDFLAGS_BATCH :=

LDFLAGS_INTERACTIVE := -L/opt/local/lib
LDFLAGS_INTERACTIVE += -lreadline

TARGET_BATCH := sudoku_batch

TARGET_INTERACTIVE := sudoku

SOURCES_BATCH := \
	grid.c \
	grid_algo.c \
	grid_pigeon.c \
	sudoku_batch.c

SOURCES_INTERACTIVE := \
	grid.c \
	grid_algo.c \
	grid_pigeon.c \
	sudoku.c

HEADERS := \
	grid.h \
	grid_internal.h

OBJECTS_BATCH := $(patsubst %.c,build/batch/%.o,$(SOURCES_BATCH))

OBJECTS_INTERACTIVE := $(patsubst %.c,build/interactive/%.o,$(SOURCES_INTERACTIVE))

DEPS := $(patsubst %.o,%.d,$(OBJECTS_BATCH) $(OBJECTS_INTERACTIVE))

SOURCES_INDENT := $(sort $(SOURCES_BATCH) $(SOURCES_INTERACTIVE) $(HEADERS))
BACKUPS_INDENT := $(addsuffix ~,$(SOURCES_INDENT))

.PHONY: all
all: $(TARGET_BATCH) $(TARGET_INTERACTIVE)

$(TARGET_BATCH): $(OBJECTS_BATCH)
	$(LD) $(LDFLAGS) $(LDFLAGS_BATCH) $(OBJECTS_BATCH) -o $(TARGET_BATCH)

$(TARGET_INTERACTIVE): $(OBJECTS_INTERACTIVE)
	$(LD) $(LDFLAGS) $(LDFLAGS_INTERACTIVE) $(OBJECTS_INTERACTIVE) -o $(TARGET_INTERACTIVE)

build/batch/%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_BATCH) -c $< -o $@

build/interactive/%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_INTERACTIVE) -c $< -o $@

$(OBJECTS_BATCH): | build/batch

$(OBJECTS_INTERACTIVE): | build/interactive

build/batch:
	mkdir -p build/batch

build/interactive:
	mkdir -p build/interactive

.PHONY: clean
clean:
	rm -rf $(TARGET_BATCH) $(TARGET_INTERACTIVE) $(TARGET_BATCH).dSYM $(TARGET_INTERACTIVE).dSYM
	rm -f $(OBJECTS_BATCH) $(OBJECTS_INTERACTIVE)
	rm -f $(DEPS)
	rm -f $(BACKUPS_INDENT)
	rmdir build/batch 2>/dev/null || :
	rmdir build/interactive 2>/dev/null || :
	rmdir build 2>/dev/null || :

.PHONY: indent
indent:
	for f in $(SOURCES_INDENT); do gindent -gnu -l120 -nut $$f; done

-include $(DEPS)
