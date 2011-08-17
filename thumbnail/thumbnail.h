#ifndef _THUMBNAIL_THUMBNAIL_H
#define _THUMBNAIL_THUMBNAIL_H

extern int thumb_width, thumb_height;

struct thumbnail {
	char *filename;
	Imlib_Image imlib;
	int width, height;
	int failed;
};

extern struct thumbnail **thumbnails;

struct thumbnail *find_thumbnail_by_name(char *);
void try_update_thumbnails();

#endif
