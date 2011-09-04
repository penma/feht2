#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "common/x11.h"

struct x11_connection *x11_connect() {
	struct x11_connection *x = malloc(sizeof(struct x11_connection));

	/* connect and cache some static values. */

	x->display  = XOpenDisplay(NULL);
	x->fd       = ConnectionNumber (x->display);
	x->screen   = DefaultScreen    (x->display);
	x->root     = DefaultRootWindow(x->display);
	x->gc       = DefaultGC        (x->display, x->screen);
	x->colormap = DefaultColormap  (x->display, x->screen);
	x->visual   = DefaultVisual    (x->display, x->screen);
	x->depth    = DefaultDepth     (x->display, x->screen);

	return x;
}

void x11_setup_imlib(struct x11_connection *x) {
	imlib_context_set_display(x->display);
	imlib_context_set_visual(x->visual);
	imlib_context_set_colormap(x->colormap);
}

Window x11_make_window(struct x11_connection *x) {
	Window win;

	XSetWindowAttributes attr;

	attr.event_mask =
		StructureNotifyMask | ExposureMask | VisibilityChangeMask |
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		PointerMotionMask | EnterWindowMask | LeaveWindowMask |
		KeyPressMask | KeyReleaseMask |
		FocusChangeMask | PropertyChangeMask;

	win = XCreateWindow(
		x->display, x->root,
		0, 0, 640, 480, 0,
		x->depth,
		InputOutput,
		x->visual,
		CWEventMask,
		&attr);

	/* map/show window */
	XMapWindow(x->display, win);

	return win;
}


