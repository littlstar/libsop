
CC ?= gcc
SRC ?= $(wildcard *.c)
SRC ?= $(wildcard ../deps/*/*.c)
TARGETS ?= $(SRC:.c=)
DEPENDENCIES ?= ../*.a

CFLAGS += -I ../include
CFLAGS += -I ../deps
CFLAGS += -l glisy
CFLAGS += -l glfw3
CFLAGS += -std=c99
CFLAGS += -Wall
CFLAGS += -framework OpenGL

$(TARGETS): $(SRC)
	$(CC) $(CFLAGS) $(DEPENDENCIES) -o $@ $@.c

.PHONY: clean
clean:
	$(RM) -f $(TARGETS)
