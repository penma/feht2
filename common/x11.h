#ifndef _SINGLE_X11_H
#define _SINGLE_X11_H

#include <X11/Xlib.h>

/* x11 connection parameters */

struct x11_connection {
	Display  *display;
	int       fd;
	int       screen;
	Window    root;
	GC        gc;
	Colormap  colormap;
	Visual   *visual;
	int       depth;
};

struct x11_connection *x11_connect();
void x11_setup_imlib(struct x11_connection *);
Window x11_make_window(struct x11_connection *);

#endif
