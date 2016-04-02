SRC += $(wildcard *.c)
SRC += $(wildcard ../deps/*/*.c)
TEST := test

CFLAGS += ../*.a
CFLAGS += -I../include
CFLAGS += -I../deps
CFLAGS += -std=c99
CFLAGS += -Wall
CFLAGS += -framework OpenGL
CFLAGS += -framework Foundation

export CFLAGS

## Compiles and runs all test suites
$(TEST): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@
	./$@

## Clean all test suites
.PHONY: clean
clean:
	$(RM) -f $(TEST)
