#ifndef COMMON_SURFACE_H
#define COMMON_SURFACE_H

#include <X11/Xlib.h>
#include <Imlib2.h>

#include "common/types.h"
#include "common/x11.h"

struct surface {
	struct x11_connection *x11;
	Window window;
	Imlib_Image imlib;
	struct coord dim; // XXX either use or remove this. (depends on how deep ev. handling goes)
};

struct surface *surface_new();
void            surface_ensure_imlib(struct surface *);
void            surface_transfer(struct surface *);

#endif
