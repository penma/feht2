all: thumbnail

CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb -I..
LDLIBS = -lX11 -lImlib2 -lm ../common/input.o ../common/x11.o

thumbnail: render.o thumbnail.o main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) thumbnail *.o

.PHONY: all clean
