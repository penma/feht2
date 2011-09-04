#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Imlib2.h>

#include "common/imlib_error.h"
#include "thumbnail/thumbnail.h"
#include "thumbnail/frame.h"
#include "thumbnail/layout.h"

extern struct layout *th_layout;
extern struct frame  *th_frame;

struct thumbnail **thumbnails = NULL;
extern int scroll_offset;

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

	struct rect on_screen = RECT(
		COORD(0, scroll_offset),
		layout->window
	);

	struct thumbnail **p = list;
	int frame_num = 0;

	while (*p != NULL) {
		struct rect frame_rect = layout_frame_rect_by_number(layout, frame_num);

		if (
			rect_intersect(frame_rect, on_screen) &&
			!(*p)->failed &&
			(
				(*p)->imlib == NULL ||
				(*p)->size  != thumb_size_for_frame(th_frame->thumb_dim)
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
				(*p)->size  != thumb_size_for_frame(th_frame->thumb_dim)
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

static Imlib_Image generate_thumbnail(const char *filename, int size) {
	fprintf(stderr, "[*] generating %dx%d thumbnail of %s\n",
		size, size, filename);

	/* load image */

	Imlib_Load_Error err;
	Imlib_Image orig = imlib_load_image_with_error_return(filename, &err);

	if (orig == NULL) {
		const char *errmsg = imlib_load_error_string(err);
		fprintf(stderr, "[-] error trying to load %s: %s\n", filename, errmsg);

		return NULL; /* XXX do what with error message? extra param? */
	}

	imlib_context_set_image(orig);

	struct coord image_dim = COORD(
		imlib_image_get_width(),
		imlib_image_get_height()
	);

	/* make thumbnail, but without upscaling smaller images. */

	struct coord thumb_dim = coord_downscale_to_fit(image_dim, COORD(size, size));

	Imlib_Image thumb = imlib_create_cropped_scaled_image(0, 0,
		image_dim.width, image_dim.height,
		thumb_dim.width, thumb_dim.height
	);

	/* can free original image now */

	imlib_context_set_image(orig);
	imlib_free_image();

	/* done */

	if (thumb == NULL) {
		fprintf(stderr, "[-] error trying to downscale %s\n", filename);

		return NULL;
	}

	return thumb;
}

int try_update_thumbnails() {
	struct thumbnail *t = next_to_update(thumbnails, th_layout);

	if (t == NULL) {
		return 0;
	}

	/* size we need */

	int size = thumb_size_for_frame(th_frame->thumb_dim);

	/* update it */

	t->imlib = generate_thumbnail(t->filename, size);

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

