#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "common/x11.h"
#include "thumbnail/thumbnail.h"

extern Window x11_window;
extern int win_width, win_height;

/* the background pixmap on which everything is drawn. */

static int pixmap_width = 0, pixmap_height = 0;
static Pixmap window_pixmap = None;

static int min(int a, int b) {
	return a < b ? a : b;
}

static void render_checkerboard() {
	/* create the checkerboard texture, if not already done. */

	static Pixmap checks_pmap = None;
	Imlib_Image checks = NULL;

	if (checks_pmap == None) {
		checks = imlib_create_image(16, 16);

		if (!checks) {
			fputs("Unable to create a teeny weeny imlib image. I detect problems\n", stderr);
		}

		imlib_context_set_image(checks);

		imlib_context_set_color(144, 144, 144, 255);
		imlib_image_fill_rectangle(0, 0, 16, 16);

		imlib_context_set_color(100, 100, 100, 255);
		imlib_image_fill_rectangle(0, 0, 8, 8);
		imlib_image_fill_rectangle(8, 8, 8, 8);

		checks_pmap = XCreatePixmap(x11.display, x11_window, 16, 16, x11.depth);

		imlib_context_set_drawable(checks_pmap);
		imlib_render_image_on_drawable(0, 0);
		imlib_free_image_and_decache();
	}

	/* and plot the checkerboards onto the pixmap */

	static GC gc = None;
	XGCValues gcval;

	if (gc == None) {
		gcval.tile = checks_pmap;
		gcval.fill_style = FillTiled;
		gc = XCreateGC(x11.display, x11_window, GCTile | GCFillStyle, &gcval);
	}

	XFillRectangle(x11.display, window_pixmap, gc, 0, 0, win_width, win_height);
}

static void ensure_window_pixmap() {
	int need_new = 0;

	/* if there's no pixmap yet, we obviously need one. */

	if (window_pixmap == None) {
		need_new = 1;
	}

	/* if window size has changed, we need a new pixmap. */

	if (win_width  != pixmap_width ||
	    win_height != pixmap_height) {
		need_new = 1;
	}

	/* now maybe create a new one */

	if (need_new) {
		if (window_pixmap != None) {
			XFreePixmap(x11.display, window_pixmap);
		}

		window_pixmap = XCreatePixmap(x11.display, x11_window, win_width, win_height, x11.depth);
		pixmap_width  = win_width;
		pixmap_height = win_height;
	}
}

void update_view() {
	/* ensure we have a sane pixmap to draw on */

	ensure_window_pixmap();

	/* render the checkerboard background */

	render_checkerboard();

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
			imlib_context_set_image(t->imlib);
			imlib_context_set_drawable(window_pixmap);
			imlib_render_image_part_on_drawable_at_size(
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

	XSetWindowBackgroundPixmap(x11.display, x11_window, window_pixmap);
	XClearWindow(x11.display, x11_window);
}

