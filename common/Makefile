CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb -I..

all: common.a

common.a: imlib_error.o x11.o input.o
	ar rcs $@ $^

clean:
	$(RM) *.o common.a

.PHONY: all clean
