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
	struct coord dim;
};

struct surface *surface_new(struct x11_connection *);
void            surface_ensure_imlib(struct surface *);
void            surface_transfer(struct surface *);

#endif
