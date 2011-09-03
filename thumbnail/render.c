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

extern Window x11_window;
extern int win_width, win_height;
extern int scroll_offset;
extern int zooming;

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

		/* we might not know the window size yet, but create
		   a window anyway. (FIXME?) */

		if (win_width == 0)
			win_width = 1;

		if (win_height == 0)
			win_height = 1;

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

	/* XXX construct layout elsewhere */

	struct frame *F = frame_new_symbols();
	F->thumbnail_size = COORD(thumb_width, thumb_height);

	struct layout *L = layout_new();
	L->window  = COORD(win_width, win_height);
	L->spacing = COORD(20, 20);
	L->frame   = F->frame_size(F);

	{
		L->frame_count = 0;
		struct thumbnail **p = thumbnails;
		while (*p != NULL) {
			p++;
			L->frame_count++;
		}
	}

	layout_recompute(L);

	/* XXX compute this or even return list from layout? */
	struct rect onscreen = RECT(
		COORD(0, scroll_offset),
		COORD(L->window.width, L->window.height)
	);

	struct thumbnail **p = thumbnails;

	int frame_num = 0;
	while (*p != NULL) {
		struct thumbnail *t = (*p);

		struct rect frame_rect = layout_frame_rect_by_number(L, frame_num);

		if (!rect_intersect(frame_rect, onscreen)) {
			/* FIXME meh, code duplication */
			p++;
			frame_num++;
			continue;
		}

		// fprintf(stderr, "[+] %s\n", (*p)->filename);

		struct rect render_rect = frame_rect;
		render_rect.topleft.y -= scroll_offset; // XXX

		F->render(F, t, render_rect);

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
	imlib_image_draw_line(0, L->total_height - scroll_offset, L->window.width, L->total_height - scroll_offset, 0);

	/* transfer the image to the window */

	imlib_context_set_image(window_imlib);
	imlib_context_set_drawable(x11_window);
	imlib_render_image_on_drawable(0, 0);
}

