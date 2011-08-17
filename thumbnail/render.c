#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "common/x11.h"
#include "thumbnail/thumbnail.h"

extern Window x11_window;
extern int win_width, win_height;

/* the background image on which everything is drawn. */

static int window_imlib_width = 0, window_imlib_height = 0;
static Imlib_Image window_imlib = NULL;

static int min(int a, int b) {
	return a < b ? a : b;
}

static void ensure_window_imlib() {
	int need_new = 0;

	/* if there's no imlib image yet, we obviously need one. */

	if (window_imlib == NULL) {
		need_new = 1;
	}

	/* if window size has changed, we need a new pixmap. */

	if (win_width  != window_imlib_width ||
	    win_height != window_imlib_height) {
		need_new = 1;
	}

	/* now maybe create a new one */

	if (need_new) {
		if (window_imlib != NULL) {
			imlib_context_set_image(window_imlib);
			imlib_free_image_and_decache();
		}

		window_imlib = imlib_create_image(win_width, win_height);
		window_imlib_width  = win_width;
		window_imlib_height = win_height;
	}
}

void update_view() {
	/* ensure we have a sane background image to draw on */

	ensure_window_imlib();

	imlib_context_set_image(window_imlib);

	/* render the background */

	/* standard black */
	imlib_context_set_color(0, 0, 0, 255);
	imlib_image_fill_rectangle(0, 0, win_width, win_height);

	/* render all the thumbnails. */

	int cell_x = 0, cell_y = 0;
	int max_cell_x = win_width / thumb_width;
	double hspacing = (win_width - (max_cell_x * thumb_width)) / (double)(max_cell_x + 1);
	double vspacing = 20.0;
	double text_height = 12.0;

	struct thumbnail **p = thumbnails;

	while (*p != NULL) {
		struct thumbnail *t = (*p);

		/* draw image, if available */

		if (t->imlib != NULL && !t->failed) {
			imlib_blend_image_onto_image(t->imlib, 1,
				/* sxywh */ 0, 0, t->width, t->height,
				/* dxywh */
					cell_x * thumb_width + hspacing * (cell_x + 1),
					cell_y * (thumb_height + text_height) + vspacing * (cell_y + 1),
					thumb_width, thumb_height
			);
		}

		/* TODO draw text */

		/* update cell pos */

		cell_x++;

		if (cell_x >= max_cell_x) {
			cell_x = 0;
			cell_y++;
		}

		p++;
	}

	/* transfer the image to the window */

	imlib_context_set_image(window_imlib);
	imlib_context_set_drawable(x11_window);
	imlib_render_image_on_drawable(0, 0);
}

