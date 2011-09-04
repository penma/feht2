#ifndef _THUMBNAIL_THUMBNAIL_H
#define _THUMBNAIL_THUMBNAIL_H

#include "common/types.h"

struct thumbnail {
	char *filename;
	Imlib_Image imlib;
	struct coord thumb_dim;
	int size;
	int failed;
};

extern struct thumbnail **thumbnails;

struct thumbnail *find_thumbnail_by_name(char *);
int try_update_thumbnails();

#endif
