## Project settings
PROJECT_NAME ?= sop
PREFIX ?= /usr/local

## Project command dependencies
MKDIR ?= mkdir -p
RM ?= rm -f
CP ?= cp -f

## Source files
SRC += $(wildcard src/*.c)

## Source objects
OBJS := $(SRC:.c=.o)

## Compiler flags
CFLAGS += -Ideps
CFLAGS += -Iinclude
CFLAGS += -Wall
CFLAGS += -O2

## Target static library
TARGET_STATIC := lib$(PROJECT_NAME).a

## Builds everything
.PHONY: all
all: $(TARGET_STATIC)

## Builds static library
$(TARGET_STATIC): $(OBJS)
	$(AR) crus $@ $^

## Compiles object files
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

## Cleans project directory
.PHONY: clean
clean: test/clean
clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET_STATIC)

## Compiles and runs all test
.PHONY: test
test: $(TARGET_STATIC)
	$(MAKE) -C $@

## Installs library into system
.PHONY: install
install: $(TARGET_STATIC)
	$(CP) -r include/* $(PREFIX)/include/
	$(CP)  $(TARGET_STATIC) $(PREFIX)/lib

## Uninstalls library from system
.PHONY: uninstall
uninstall:
	$(RM) -r $(PREFIX)/include/$(PROJECT_NAME)
	$(RM) $(PREFIX)/lib/$(TARGET_STATIC)

## Cleans test directory
.PHONY: test/clean
test/clean:
	$(MAKE) clean -C test
