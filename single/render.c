#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "state.h"

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

		checks_pmap = XCreatePixmap(s_x11.display, s_x11.window, 16, 16, s_x11.depth);

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
		gc = XCreateGC(s_x11.display, s_x11.window, GCTile | GCFillStyle, &gcval);
	}

	XFillRectangle(s_x11.display, window_pixmap, gc, 0, 0, s_view.win_width, s_view.win_height);
}

static void ensure_window_pixmap() {
	int need_new = 0;

	/* if there's no pixmap yet, we obviously need one. */

	if (window_pixmap == None) {
		need_new = 1;
	}

	/* if window size has changed, we need a new pixmap. */

	if (s_view.win_width  != pixmap_width ||
	    s_view.win_height != pixmap_height) {
		need_new = 1;
	}

	/* now maybe create a new one */

	if (need_new) {
		if (window_pixmap != None) {
			XFreePixmap(s_x11.display, window_pixmap);
		}

		window_pixmap = XCreatePixmap(s_x11.display, s_x11.window, s_view.win_width, s_view.win_height, s_x11.depth);
		pixmap_width  = s_view.win_width;
		pixmap_height = s_view.win_height;
	}
}

void render_image() {
	/* ensure we have a sane pixmap to draw on */

	ensure_window_pixmap();

	/* render the checkerboard background */

	render_checkerboard();

	/* rendering the image, if we have one. */

	if (s_image.imlib != NULL) {
		/* calculate draw offsets. the pan offsets define the location of
		   the top left corner on the window, regardless of scaling. */

		int sx, sy, dx, dy, sw, sh, dw, dh;

		if (s_view.pan_x < 0) {
			/* left part of image is hidden */
			sx = -s_view.pan_x;
			dx = 0;
		} else {
			/* left part of image is somewhere in the window */
			sx = 0;
			dx = s_view.pan_x;
		}

		if (s_view.pan_y < 0) {
			/* top part of image is hidden */
			sy = -s_view.pan_y;
			dy = 0;
		} else {
			/* top part of image is somewhere in the window */
			sy = 0;
			dy = s_view.pan_y;
		}

		/* how much of the image is visible? */

		dw = min(s_view.win_width  - dx, s_image.width  - sx);
		dh = min(s_view.win_height - dy, s_image.height - sy);
		sw = dw; sh = dh;

		fprintf(stderr, "now rendering to coordinates s=%d,%d/%dx%d d=%d,%d/%dx%d (scale: %f)\n", sx, sy, sw, sh, dx, dy, dw, dh, s_view.scale);

		/* now we know all the coordinates, we can render the image */

		imlib_context_set_image(s_image.imlib);
		imlib_context_set_drawable(window_pixmap);
		imlib_render_image_part_on_drawable_at_size(
			/* sxywh */ sx, sy, sw, sh,
			/* dxywh */ dx, dy, dw, dh
		);
	} else {
		fputs("not rendering because there is no image.\n", stderr);
	}

	XSetWindowBackgroundPixmap(s_x11.display, s_x11.window, window_pixmap);
	XClearWindow(s_x11.display, s_x11.window);

	XFlush(s_x11.display);
}

