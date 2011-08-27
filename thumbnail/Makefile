all: thumbnail

CC = clang
CFLAGS = -std=c99 -Wall -Wextra -ggdb -I..
LDLIBS = -lX11 -lImlib2 -lm ../common/common.a

thumbnail: render.o thumbnail.o main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) thumbnail *.o

.PHONY: all clean
