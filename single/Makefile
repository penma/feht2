all: single

CC = clang
CFLAGS = -Wall -Wextra -ggdb -I..
LDLIBS = -lX11 -lImlib2 -lm ../common/input.o ../common/x11.o

single: ctl.o render.o single.o

clean:
	$(RM) single *.o

.PHONY: all clean
