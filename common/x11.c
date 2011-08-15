#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "common/x11.h"
#include "single/state.h"

struct x11_connection x11;

void setup_x11() {
	/* connect and cache some static values. */

	x11.display  = XOpenDisplay(NULL);
	x11.fd       = ConnectionNumber (x11.display);
	x11.screen   = DefaultScreen    (x11.display);
	x11.root     = DefaultRootWindow(x11.display);
	x11.gc       = DefaultGC        (x11.display, x11.screen);
	x11.colormap = DefaultColormap  (x11.display, x11.screen);
	x11.visual   = DefaultVisual    (x11.display, x11.screen);
	x11.depth    = DefaultDepth     (x11.display, x11.screen);
}

void setup_imlib() {
	imlib_context_set_display(x11.display);

	imlib_context_set_visual(x11.visual);
	imlib_context_set_colormap(x11.colormap);
}

/* unknown */ void make_window() {
	XSetWindowAttributes attr;

	attr.event_mask =
		StructureNotifyMask | ExposureMask | VisibilityChangeMask |
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		PointerMotionMask | EnterWindowMask | LeaveWindowMask |
		KeyPressMask | KeyReleaseMask |
		FocusChangeMask | PropertyChangeMask;

	x11_window = XCreateWindow(
		x11.display, x11.root,
		0, 0, 640, 480, 0,
		x11.depth,
		InputOutput,
		x11.visual,
		CWEventMask,
		&attr);

	/* map/show window */
	XMapWindow(x11.display, x11_window);

	/* query real size */
	XWindowAttributes xwa;
	XGetWindowAttributes(x11.display, x11_window, &xwa);
	s_view.win_width  = xwa.width;
	s_view.win_height = xwa.height;
}


