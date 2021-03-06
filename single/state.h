#ifndef _SINGLE_STATE_H
#define _SINGLE_STATE_H

#include <X11/Xlib.h>
#include <Imlib2.h>

/* x11 state */

extern Window x11_window;

/* image view parameters */

struct _single_state_view {
	int pan_x, pan_y;
	double scale;
	double angle;

	int win_width, win_height;

	int dirty;
};

extern struct _single_state_view s_view;

/* image data */

struct _single_state_image {
	const char *filename;
	Imlib_Image imlib;
	int width, height;
};

extern struct _single_state_image s_image;

#endif
