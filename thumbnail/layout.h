#ifndef _THUMBNAIL_LAYOUT_H
#define _THUMBNAIL_LAYOUT_H

#include "common/types.h"

struct layout {
	struct coord
		window,  /* window dimensions */
		border,  /* border around windows */
		spacing, /* spacing between frames */
		frame;   /* frame dimension (w = minimal) */
	int frame_count;

	int _frames_per_row;
	double _frame_spacing;
};

struct layout *layout_new();
int            layout_frame_number_by_coordinate(struct layout *, struct coord);
struct rect    layout_frame_rect_by_number(struct layout *, int);
void           layout_recompute(struct layout *);

#endif
