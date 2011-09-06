#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <Imlib2.h>

#include "thumbnail/view.h"

#include "thumbnail/thumbnail.h"
extern int zooming;


struct view *view_new() {
	struct view *v = malloc(sizeof(struct view));

	v->surface = NULL;
	v->layout = NULL;
	v->frame = NULL;
	v->scroll_offset = 0;

	return v;
}

struct rect view_visible_rect(struct view *v) {
	return RECT(
		COORD(0, v->scroll_offset),
		v->layout->window
	);
}

void view_render(struct view *v) {
	/* ensure we have a sane background image to draw on */

	surface_ensure_imlib(v->surface);

	imlib_context_set_image(v->surface->imlib);

	/* render the background */

	/* standard black */
	imlib_context_set_color(0, 0, 0, 255);
	imlib_image_fill_rectangle(0, 0, v->layout->window.width, v->layout->window.height);

	/* render all the thumbnails. */

	struct rect onscreen = view_visible_rect(v);

	struct thumbnail **p = thumbnails;

	int frame_num = 0;
	for (; *p != NULL; (p++, frame_num++)) {
		struct thumbnail *t = (*p);

		struct rect frame_rect = layout_frame_rect_by_number(v->layout, frame_num);

		if (!rect_intersect(frame_rect, onscreen)) {
			continue;
		}

		// fprintf(stderr, "[+] %s\n", (*p)->filename);

		struct rect render_rect = frame_rect;
		render_rect.tl.y -= v->scroll_offset; // XXX

		v->frame->render(v->frame, t, render_rect);

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
	}

	imlib_context_set_color(255, 255, 255, 255);
	imlib_image_draw_line(
		0, v->layout->total_height - v->scroll_offset,
		v->layout->window.width, v->layout->total_height - v->scroll_offset,
	0);

	/* transfer the image to the window */

	surface_transfer(v->surface);
}

