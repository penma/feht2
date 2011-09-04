#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "common/x11.h"
#include "thumbnail/thumbnail.h"
#include "thumbnail/layout.h"
#include "thumbnail/frame.h"

extern struct layout *th_layout;
extern struct frame  *th_frame;

extern int scroll_offset;
extern int zooming;

/* the background image on which everything is drawn. */

static struct coord window_imlib_dim;
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

	if (th_layout->window.width  != window_imlib_dim.width ||
	    th_layout->window.height != window_imlib_dim.height) {
		need_new = 1;
	}

	/* now maybe create a new one */

	if (need_new) {
		if (window_imlib != NULL) {
			imlib_context_set_image(window_imlib);
			imlib_free_image_and_decache();
		}

		/* we might not know the window size yet, but create
		   a window anyway. (FIXME?) */

		window_imlib_dim = th_layout->window;

		if (window_imlib_dim.width == 0)
			window_imlib_dim.width = 1;

		if (window_imlib_dim.height == 0)
			window_imlib_dim.height = 1;

		window_imlib = imlib_create_image(window_imlib_dim.width, window_imlib_dim.height);
	}
}

void update_view() {
	/* ensure we have a sane background image to draw on */

	ensure_window_imlib();

	imlib_context_set_image(window_imlib);

	/* render the background */

	/* standard black */
	imlib_context_set_color(0, 0, 0, 255);
	imlib_image_fill_rectangle(0, 0, th_layout->window.width, th_layout->window.height);

	/* render all the thumbnails. */

	/* XXX compute this or even return list from layout? */
	struct rect onscreen = RECT(
		COORD(0, scroll_offset),
		th_layout->window
	);

	struct thumbnail **p = thumbnails;

	int frame_num = 0;
	while (*p != NULL) {
		struct thumbnail *t = (*p);

		struct rect frame_rect = layout_frame_rect_by_number(th_layout, frame_num);

		if (!rect_intersect(frame_rect, onscreen)) {
			/* FIXME meh, code duplication */
			p++;
			frame_num++;
			continue;
		}

		// fprintf(stderr, "[+] %s\n", (*p)->filename);

		struct rect render_rect = frame_rect;
		render_rect.topleft.y -= scroll_offset; // XXX

		th_frame->render(th_frame, t, render_rect);

		/* FIXME (global vars etc)
		   when resizing thumbs, draw borders to make the change more
		   easily visible. */

		if (zooming) {
			imlib_image_draw_rectangle(
				render_rect.topleft.x,
				render_rect.topleft.y,
				render_rect.dimensions.x,
				render_rect.dimensions.y
			);
		}

		p++;
		frame_num++;
	}

	imlib_context_set_color(255, 255, 255, 255);
	imlib_image_draw_line(
		0, th_layout->total_height - scroll_offset,
		th_layout->window.width, th_layout->total_height - scroll_offset,
	0);

	/* transfer the image to the window */

	imlib_context_set_image(window_imlib);
	imlib_context_set_drawable(x11.window);
	imlib_render_image_on_drawable(0, 0);
}

