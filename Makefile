all: single/single

CC = clang
CFLAGS = -Wall -Wextra -ggdb -I.
LIBS = -lX11 -lImlib2

single/single:
	$(CC) $(CFLAGS) -o single/single single/*.c $(LIBS)

clean:
	$(RM) single/single

.PHONY: all single/single clean
