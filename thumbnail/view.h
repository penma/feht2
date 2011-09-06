#ifndef THUMBNAIL_VIEW_H
#define THUMBNAIL_VIEW_H

#include <X11/Xlib.h>
#include <Imlib2.h>

#include "common/surface.h"
#include "thumbnail/layout.h"
#include "thumbnail/frame.h"

/* XXX where should window dimensions be ultimately maintained? */

struct view {
	struct surface *surface;
	struct layout *layout;
	struct frame *frame;

	int scroll_offset; /* TODO more stuff for retaining a more natural (%) offset? */
};

struct view *view_new();
struct rect view_visible_rect(struct view *);
void view_render(struct view *);

#endif
