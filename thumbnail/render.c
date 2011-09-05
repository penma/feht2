#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "thumbnail/thumbnail.h"
#include "thumbnail/layout.h"
#include "thumbnail/frame.h"
#include "common/surface.h"

extern struct surface *surf;
extern struct layout *th_layout;
extern struct frame  *th_frame;

extern int scroll_offset;
extern int zooming;

/* the background image on which everything is drawn. */

static int min(int a, int b) {
	return a < b ? a : b;
}

void update_view() {
	/* ensure we have a sane background image to draw on */

	surface_ensure_imlib(surf);

	imlib_context_set_image(surf->imlib);

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
		render_rect.tl.y -= scroll_offset; // XXX

		th_frame->render(th_frame, t, render_rect);

		/* FIXME (global vars etc)
		   when resizing thumbs, draw borders to make the change more
		   easily visible. */

		if (zooming) {
			imlib_image_draw_rectangle(
				render_rect.tl.x,
				render_rect.tl.y,
				render_rect.dim.x,
				render_rect.dim.y
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

	surface_transfer(surf);
}

