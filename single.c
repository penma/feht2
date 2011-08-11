#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

#include <Imlib2.h>

void spam(const char *foo) {
	write(2, foo, strlen(foo));
}

Imlib_Image *current_image = NULL;

int load_image(char *filename) {
	if (current_image != NULL) {
		imlib_context_set_image(current_image);
		imlib_free_image();
	}

	/* TODO error handling */
	current_image = imlib_load_image(filename);

	return 0;
}

void init_imlib_on_display(Display *dpy) {
	imlib_context_set_display(dpy);

	imlib_context_set_visual(DefaultVisual(dpy, DefaultScreen(dpy)));
	imlib_context_set_colormap(DefaultColormap(dpy, DefaultScreen(dpy)));

	imlib_context_set_color_modifier(NULL); /* magic or necessary? */
	imlib_context_set_progress_function(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);
}

Window make_window(Display *dpy) {
	XSetWindowAttributes attr;

	int scr = DefaultScreen(dpy);

	attr.backing_store = NotUseful;
	attr.override_redirect = False;
	attr.colormap = DefaultColormap(dpy, scr);
	attr.border_pixel = 0;
	attr.background_pixel = 0;
	attr.save_under = False;
	attr.event_mask =
		StructureNotifyMask | ExposureMask | VisibilityChangeMask |
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		PointerMotionMask | EnterWindowMask | LeaveWindowMask |
		KeyPressMask | KeyReleaseMask |
		FocusChangeMask | PropertyChangeMask;

	Window win = XCreateWindow(
		dpy, DefaultRootWindow(dpy),
		0, 0, 640, 480, 0,
		DefaultDepth(dpy, scr),
		InputOutput,
		DefaultVisual(dpy, scr),
		CWBackingStore | CWOverrideRedirect | CWColormap |
		CWBorderPixel | CWBackPixel | CWSaveUnder |
		CWEventMask,
		&attr);

	/* map/show window */
	XMapWindow(dpy, win);

	return win;
}

void event_handle_ctl(int fd) {
	/* TODO actually handle commands. */

	/* TODO implement a buffering system */


	/* HACK: just "empty" control fd. */
	char foo[4096];
	read(fd, foo, 4096);
}

void event_handle_x11(Display *dpy) {
	XEvent ev;

	while (XPending(dpy)) {
		XNextEvent(dpy, &ev);
	}

	XFlush(dpy);
}

void event_loop(Display *dpy, int ctl_fd) {
	int x_fd = ConnectionNumber(dpy);
	int max_fd = (x_fd > ctl_fd ? x_fd : ctl_fd) + 1;

	fd_set fds;
	int ret;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(x_fd, &fds);
		FD_SET(ctl_fd, &fds);

		spam("wait for something to happen.\n");
		ret = select(max_fd, &fds, NULL, NULL, NULL);

		if (ret == -1) {
			perror("select"); /* XXX */
		}

		if (FD_ISSET(ctl_fd, &fds)) {
			spam("handle command\n");
			event_handle_ctl(ctl_fd);
		}

		if (FD_ISSET(x_fd, &fds)) {
			spam("handle X events\n");
			event_handle_x11(dpy);
		}
	}
}

int main(int argc, char *argv[]) {
	/* TODO parse options */
	if (argc != 2) {
		spam("provide exactly one argument!\n");
		exit(1);
	}

	/* TODO init imlib */
	Display *dpy = XOpenDisplay(NULL);
	init_imlib_on_display(dpy);

	/* make window */
	Window win = make_window(dpy);

	Pixmap bg_pmap = XCreatePixmap(dpy, win, 256, 256, 24);

	/* load image */
	load_image(argv[1]);

	/* render image */
	imlib_context_set_image(current_image);
	imlib_context_set_drawable(bg_pmap);
	imlib_render_image_part_on_drawable_at_size(
		/* sxywh */ 0, 0, 256, 256,
		/* dxywh */ 0, 0, 256, 256
	);

	XSetWindowBackgroundPixmap(dpy, win, bg_pmap);
	XClearWindow(dpy, win);

	XFlush(dpy);

	event_loop(dpy, 0); /* 0 = stdin */
}

