#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Imlib2.h>

#include "common/imlib_error.h"
#include "thumbnail/thumbnail.h"
#include "thumbnail/nailer.h"
#include "thumbnail/frame.h"
#include "thumbnail/layout.h"
#include "thumbnail/view.h"

struct thumbnail **thumbnails = NULL;
extern struct view *view;

static int max(int a, int b) {
	return a > b ? a : b;
}

struct thumbnail *find_thumbnail_by_name(char *filename) {
	struct thumbnail **p = thumbnails;
	while (*p != NULL) {
		if (!strcmp(filename, (*p)->filename)) {
			return *p;
		}
		p++;
	}
	return NULL;
}

static int thumb_size_for_frame(struct coord frame_dim) {
	int longer = max(frame_dim.width, frame_dim.height);

	if (longer <= 128) {
		return 128;
	} else if (longer <= 256) {
		return 256;
	} else {
		return 512;
	}
}

static struct thumbnail *next_to_update(struct thumbnail **list, struct layout *layout) {
	if (list == NULL) {
		return NULL;
	}

	/* we first check all those that are in view. */

	struct rect onscreen = view_visible_rect(view);

	struct thumbnail **p = list;
	int frame_num = 0;

	while (*p != NULL) {
		struct rect frame_rect = layout_frame_rect_by_number(layout, frame_num);

		if (
			rect_intersect(frame_rect, onscreen) &&
			!(*p)->failed &&
			(
				(*p)->imlib == NULL ||
				(*p)->size  != thumb_size_for_frame(view->frame->thumb_dim)
			)
		) {
			return (*p);
		}

		p++;
		frame_num++;
	}

	/* then all the others */

	p = list;
	frame_num = 0;

	while (*p != NULL) {
		if (
			!(*p)->failed &&
			(
				(*p)->imlib == NULL ||
				(*p)->size  != thumb_size_for_frame(view->frame->thumb_dim)
			)
		) {
			return (*p);
		}

		p++;
		frame_num++;
	}

	/* none! */

	return NULL;
}

int try_update_thumbnails() {
	struct thumbnail *t = next_to_update(thumbnails, view->layout);

	if (t == NULL) {
		return 0;
	}

	/* size we need */

	int size = thumb_size_for_frame(view->frame->thumb_dim);

	/* update it */

	t->imlib = thumbnail_from_storage(t->filename, size);

	if (t->imlib != NULL) {
		imlib_context_set_image(t->imlib);
		t->thumb_dim = COORD(
			imlib_image_get_width(),
			imlib_image_get_height()
		);
		t->size = size;
	} else {
		t->failed = 1;
	}


	/* update no more than one */

	return 1;
}

