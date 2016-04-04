
OS ?= $(shell uname)
CC ?= gcc

SRC = $(wildcard *.c)
SRC += $(wildcard ../deps/*/*.c)


TARGETS ?= $(SRC:.c=)
DEPENDENCIES ?= ../*.a

CFLAGS += -I ../include
CFLAGS += -I ../deps
CFLAGS += -l glfw3
CFLAGS += -std=c99
CFLAGS += -Wall

ifeq (Darwin, $(OS))
CFLAGS += -framework OpenGL
CFLAGS += -framework Foundation
endif


$(TARGETS): $(SRC)
	$(CC) $(CFLAGS) $(DEPENDENCIES) -o $@ $^

.PHONY: clean
clean:
	$(RM) -f $(TARGETS)
