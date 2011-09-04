#ifndef _THUMBNAIL_FRAME_H
#define _THUMBNAIL_FRAME_H

#include "common/types.h"

#include "thumbnail/thumbnail.h"

struct frame {
	struct coord thumb_dim;

	void (*render)(struct frame *, struct thumbnail *, struct rect);
	struct coord (*frame_size)(struct frame *);
};

struct frame *frame_new_symbols();

#endif
