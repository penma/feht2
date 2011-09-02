all: thumbnail/thumbnail

OBJ_COMMON    = $(addprefix common/,imlib_error.o x11.o input.o types.o)
OBJ_THUMBNAIL = $(addprefix thumbnail/,layout.o frame.o render.o thumbnail.o main.o)

CFLAGS = -std=c99 -Wall -Wextra -ggdb -I.

thumbnail/thumbnail: $(OBJ_COMMON) $(OBJ_THUMBNAIL)
	$(CC) $(LDFLAGS) -o $@ $^ -lX11 -lImlib2 -lm

clean:
	$(RM) common/*.o thumbnail/*.o thumbnail/thumbnail

.PHONY: all clean
