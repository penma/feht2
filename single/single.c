#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

#include <Imlib2.h>

#include "single/crap.h"
#include "single/ctl.h"

struct {
	Display *display;
	int fd;
	GC gc;
	int screen;
	Colormap colormap;
	Visual *visual;
	int depth;
	Window window;
	Pixmap pixmap;
} x11;

const char *imliberr(Imlib_Load_Error e) {
	switch (e) {
	case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST                : return "No such file or directory";
	case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY                  : return "Is a directory";
	case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ          : return "Permission denied";
	case IMLIB_LOAD_ERROR_UNKNOWN                            :
	case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT          : return "No Imlib2 loader for that file format";
	case IMLIB_LOAD_ERROR_PATH_TOO_LONG                      : return "Filename too long";
	case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT        : return "No such file or directory (along path)";
	case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY       : return "Not a directory";
	case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE  : return "Bad address";
	case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS            : return "Too many levels of symbolic links";
	case IMLIB_LOAD_ERROR_OUT_OF_MEMORY                      : return "Out of memory";
	case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS            : return "Too many open files";
	case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE         : return "Permission denied (to write to directory)";
	case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE                  : return "No space left on device";
	default                                                  : return "Unknown error";
	}
}

Imlib_Image *current_image = NULL;

int load_image(char *filename) {
	if (current_image != NULL) {
		imlib_context_set_image(current_image);
		imlib_free_image();
	}

	/* TODO error handling */
	Imlib_Load_Error err;
	current_image = imlib_load_image_with_error_return(filename, &err);

	if (current_image == NULL) {
		const char *errmsg = imliberr(err);
		spam("imlib load error: ");
		spam(errmsg);
		spam("\n");
	}

	return 0;
}

int render_image_primitive() {
	spam("rendering\n");

	imlib_context_set_image(current_image);
	imlib_context_set_drawable(x11.pixmap);
	imlib_render_image_part_on_drawable_at_size(
		/* sxywh */ 0, 0, 256, 256,
		/* dxywh */ 0, 0, 256, 256
	);

	XSetWindowBackgroundPixmap(x11.display, x11.window, x11.pixmap);
	XClearWindow(x11.display, x11.window);

	XFlush(x11.display);
}

static void init_imlib() {
	imlib_context_set_display(x11.display);

	imlib_context_set_visual(x11.visual);
	imlib_context_set_colormap(x11.colormap);

	imlib_context_set_color_modifier(NULL); /* magic or necessary? */
	imlib_context_set_progress_function(NULL);
	imlib_context_set_operation(IMLIB_OP_COPY);
}

static void make_window() {
	XSetWindowAttributes attr;

	attr.event_mask =
		StructureNotifyMask | ExposureMask | VisibilityChangeMask |
		ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		PointerMotionMask | EnterWindowMask | LeaveWindowMask |
		KeyPressMask | KeyReleaseMask |
		FocusChangeMask | PropertyChangeMask;

	x11.window = XCreateWindow(
		x11.display, DefaultRootWindow(x11.display),
		0, 0, 640, 480, 0,
		x11.depth,
		InputOutput,
		x11.visual,
		CWEventMask,
		&attr);

	/* map/show window */
	XMapWindow(x11.display, x11.window);
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
	int max_fd = (x11.fd > ctl_fd ? x11.fd : ctl_fd) + 1;

	fd_set fds;
	int ret;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(x11.fd, &fds);
		FD_SET(ctl_fd, &fds);

		spam("wait for something to happen.\n");
		ret = select(max_fd, &fds, NULL, NULL, NULL);

		if (ret == -1) {
			perror("select"); /* XXX */
		}

		if (FD_ISSET(ctl_fd, &fds)) {
			spam("handle command\n");
			ctl_handle_fd(ctl_fd);
		}

		if (FD_ISSET(x11.fd, &fds)) {
			spam("handle X events\n");
			event_handle_x11(dpy);
		}
	}
}

int main(int argc, char *argv[]) {
	/* TODO parse options */
	if (argc != 1) {
		spam("single does not take arguments. images are loaded via the control channel\n");
		exit(1);
	}

	/* TODO init imlib */
	x11.display  = XOpenDisplay(NULL);
	x11.fd       = ConnectionNumber(x11.display);
	x11.screen   = DefaultScreen(x11.display);
	x11.gc       = DefaultGC(x11.display, x11.screen);
	x11.colormap = DefaultColormap(x11.display, x11.screen);
	x11.visual   = DefaultVisual(x11.display, x11.screen);
	x11.depth    = DefaultDepth(x11.display, x11.screen);

	init_imlib();

	/* make window */
	make_window();
	x11.pixmap = XCreatePixmap(x11.display, x11.window, 256, 256, 24);

	XFlush(x11.display);

	event_loop(x11.display, 0); /* 0 = stdin */
}

