all: thumbnail/thumbnail

OBJ_COMMON    = $(addprefix common/,imlib_error.o x11.o input.o surface.o types.o)
OBJ_THUMBNAIL = $(addprefix thumbnail/,layout.o frame.o view.o feh_png.o nailer.o thumbnail.o main.o)

CFLAGS = -D_GNU_SOURCE -std=c99 -Wall -Wextra -ggdb -I.

thumbnail/thumbnail: $(OBJ_COMMON) $(OBJ_THUMBNAIL)
	$(CC) $(LDFLAGS) -o $@ $^ -lX11 -lImlib2 -lbsd -lgiblib -lpng -lm

clean:
	$(RM) common/*.o thumbnail/*.o thumbnail/thumbnail

.PHONY: all clean
