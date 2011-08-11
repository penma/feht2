all: single

CC = clang
CFLAGS = -Wall -Wextra -ggdb
LIBS = -lX11 -lImlib2

single:
	$(CC) $(CFLAGS) -o single single.c $(LIBS)

clean:
	$(RM) single

.PHONY: all single clean
