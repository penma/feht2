#include <stdio.h>

#include "thumbnail/layout.h"

struct layout *layout_new() {
	struct layout *l = malloc(sizeof(struct layout));

	l->frame_count = 0;
	l->window  = COORD(0, 0);
	l->border  = COORD(0, 0);
	l->spacing = COORD(0, 0);
	l->frame   = COORD(0, 0);

	return l;
}

int layout_frame_number_by_coordinate(struct layout *l, struct coord probe) {
	/* FIXME use a more efficient implementation. */

	for (int f = 0; f < l->frame_count; f++) {
		struct rect ff = layout_frame_rect_by_number(l, f);
		fprintf(stderr, "test f %d r %d/%d %dx%d in %d,%d\n",
			f,
			ff.topleft.x, ff.topleft.x,
			ff.dimensions.width, ff.dimensions.height,
			probe.x, probe.y
		);

		if (
			ff.topleft.x <= probe.x &&
			probe.x <= ff.topleft.x + ff.dimensions.width &&
			ff.topleft.y <= probe.y &&
			probe.y <= ff.topleft.y + ff.dimensions.height
		) {
			return f;
		}
	}

	return -1;
}

struct rect layout_frame_rect_by_number(struct layout *l, int frame) {
	/* refuse to calculate coordinates for a frame that doesn't exist,
	   even though we could.
	   this makes it consistent with layout_frame_number_by_coordinate,
	   which does need to respect the frame count, to prevent returning
	   an invalid frame id. */

	if (frame > l->frame_count) {
		return RECT(COORD(0, 0), COORD(0, 0));
	}

	struct rect r;

	int col = frame % l->_frames_per_row;
	int row = frame / l->_frames_per_row;

	r.topleft.x = l->border.horizontal
	            +  col      * l->_frame_layout_width
	            + (col - 1) * l->spacing.horizontal;
	r.topleft.y = l->border.vertical
	            +  row      * l->frame.height
	            + (row - 1) * l->spacing.vertical;

	/* dimensions of the frame
	   width is computed, height is predefined. */

	r.dimensions = COORD(l->_frame_width, l->frame.height);

	return r;
}

void layout_recompute(struct layout *l) {
	/* calculate the real frame sizes.
	   as the .frame field only specifies a minimal width we need to
	   calculate the real available width given a fixed spacing.
	   frame width should be the same for all frames and not vary every
	   second frame because of rounding. better increase the spacing.
	   frame layout width is used to calculate horizontal position of
	   a frame, and therefore is floating point. */

	int main_width = l->window.width
	           - 2 * l->border.horizontal
	           +     l->spacing.horizontal;
	int fpr = main_width / (l->frame.width + l->spacing.horizontal);
	l->_frame_layout_width = (main_width - l->spacing.horizontal * fpr) / (double)fpr;
	l->_frame_width = l->_frame_layout_width;
	l->_frames_per_row = fpr;

	/* the above produces invalid numbers if the window is too narrow. */

	if (fpr == 0) {
		l->_frames_per_row = 1;
		l->_frame_layout_width = l->window.width;
		l->_frame_width = l->window.width;
	}
}
