all: single

CC = clang
CFLAGS = -Wall -Wextra -ggdb -I.
LDLIBS = -lX11 -lImlib2

single: ctl.o input.o render.o single.o

clean:
	$(RM) single *.o

.PHONY: all clean
