#ifndef _SINGLE_X11_H
#define _SINGLE_X11_H

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

extern struct x11_connection x11;

void setup_x11();
void setup_imlib();

#endif
