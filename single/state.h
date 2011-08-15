#ifndef _SINGLE_STATE_H
#define _SINGLE_STATE_H

#include <X11/Xlib.h>
#include <Imlib2.h>

/* x11 state */

struct _single_state_x11 {
	Display *display;
	int fd;
	GC gc;
	int screen;
	Colormap colormap;
	Visual *visual;
	int depth;
	Window window;
	Pixmap pixmap;
};

extern struct _single_state_x11 s_x11;

/* image view parameters */

struct _single_state_view {
	int pan_x, pan_y;
	double scale;
	double angle;

	int win_width, win_height;
};

extern struct _single_state_view s_view;

/* input state */

struct _single_state_input {
	int panning;
	int pan_last_x, pan_last_y;
};

extern struct _single_state_input s_input;

/* image data */

struct _single_state_image {
	const char *filename;
	Imlib_Image imlib;
	int width, height;
};

extern struct _single_state_image s_image;

#endif
