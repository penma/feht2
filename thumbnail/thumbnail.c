#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Imlib2.h>

#include "thumbnail/thumbnail.h"

const char *imliberr(Imlib_Load_Error e) {
	switch (e) {
	case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST                : return "No such file or directory";
	case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY                  : return "Is a directory";
	case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ          : return "Permission denied";
	case IMLIB_LOAD_ERROR_UNKNOWN                            :
	case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT          : return "No Imlib2 loader for that file format";
	case IMLIB_LOAD_ERROR_PATH_TOO_LONG                      : return "Filename too long";
	case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT        : return "No such file or directory (along path)";
	case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY       : return "Not a directory";
	case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE  : return "Bad address";
	case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS            : return "Too many levels of symbolic links";
	case IMLIB_LOAD_ERROR_OUT_OF_MEMORY                      : return "Out of memory";
	case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS            : return "Too many open files";
	case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE         : return "Permission denied (to write to directory)";
	case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE                  : return "No space left on device";
	default                                                  : return "Unknown error";
	}
}

int thumb_width = 200, thumb_height = 150;
struct thumbnail **thumbnails = NULL;

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

void try_update_thumbnails() {
	if (thumbnails == NULL) {
		return;
	}

	struct thumbnail **p = thumbnails;

	while (*p != NULL) {
		if (!(*p)->failed && (
		    (*p)->imlib == NULL ||
		    (*p)->width != thumb_width ||
		    (*p)->height != thumb_height)) {
			/* update */

			struct thumbnail *t = (*p);

			fprintf(stderr, "trying to update thumbnail for %s\n", t->filename);

			if (t->imlib != NULL) {
				imlib_context_set_image(t->imlib);
				imlib_free_image_and_decache();
			}

			Imlib_Load_Error err;
			Imlib_Image orig = imlib_load_image_with_error_return(t->filename, &err);

			if (orig == NULL) {
				const char *errmsg = imliberr(err);
				fprintf(stderr, "error trying to load %s: %s\n", t->filename, errmsg);
				t->failed = 1;

				return;
			}

			imlib_context_set_image(orig);
			int width  = imlib_image_get_width();
			int height = imlib_image_get_height();

			/* compute size and position of the preview inside the thumb frame */

			int dw, dh, dx, dy;

			dw = width < thumb_width ? width : thumb_width;
			dh = dw * (double)height / width;

			if (dh > thumb_height) {
				dh = thumb_height;
				dw = dh * (double)width / height;
			}

			dx = (thumb_width  - dw) / 2;
			dy = (thumb_height - dh) / 2;

			/* create the thumbnail */

			t->imlib = imlib_create_image(thumb_width, thumb_height);
			imlib_context_set_image(t->imlib);
			imlib_image_set_has_alpha(1);

			imlib_context_set_blend(0);
			imlib_context_set_color(0, 0, 0, 0);
			imlib_image_fill_rectangle(0, 0, thumb_width, thumb_height);
			imlib_context_set_blend(1);

			imlib_blend_image_onto_image(
				orig, 1,
				0, 0, width, height,
				dx, dy, dw, dh
			);

			t->width  = thumb_width;
			t->height = thumb_height;

			/* can free original image now */

			imlib_context_set_image(orig);
			imlib_free_image();

			/* update no more than one */

			return;
		}

		p++;
	}
}

