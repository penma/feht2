#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "thumbnail/layout.h"

struct layout *layout_new() {
	struct layout *l = malloc(sizeof(struct layout));

	l->frame_count = 0;
	l->window  = COORD(0, 0);
	l->spacing = COORD(0, 0);
	l->frame   = COORD(0, 0);

	return l;
}

int layout_frame_number_by_coordinate(struct layout *l, struct coord probe) {
	/* FIXME use a more efficient implementation. */

	for (int f = 0; f < l->frame_count; f++) {
		struct rect ff = layout_frame_rect_by_number(l, f);

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

	if (frame >= l->frame_count) {
		return RECT(COORD(0, 0), COORD(0, 0));
	}

	struct rect r;

	int col = frame % l->_frames_per_row;
	int row = frame / l->_frames_per_row;

	r.topleft.x = (col + 1) * l->_frame_spacing
	            +  col      * l->frame.width;
	r.topleft.y = (row + 1) * l->spacing.vertical
	            +  row      * l->frame.height;

	r.dimensions = l->frame;

	return r;
}

void layout_recompute(struct layout *l) {
	/* calculate frame spacing.
	   in the horizontal direction, spacing specifies a minimal spacing
	   but it may be larger because the window can't fit another full
	   frame. */

	int fpr = (l->window.width - l->spacing.horizontal)
	        / (l->frame.width  + l->spacing.horizontal);

	if (fpr < 1) {
		fpr = 1;
	}

	l->_frame_spacing = (l->window.width - l->frame.width * fpr) / (double)(fpr + 1);
	l->_frames_per_row = fpr;

	/* total height */

	int rows = ceil(l->frame_count / (double)fpr);

	l->total_height =  rows      * l->frame.height
	                + (rows + 1) * l->spacing.vertical;
}
