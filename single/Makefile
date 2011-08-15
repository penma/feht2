all: single

CC = clang
CFLAGS = -Wall -Wextra -ggdb
LDLIBS = -lX11 -lImlib2

single: ctl.o input.o render.o single.o x11.o

clean:
	$(RM) single *.o

.PHONY: all clean
