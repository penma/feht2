#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

#include <Imlib2.h>

#include "ctl.h"
#include "input.h"
#include "state.h"
#include "render.h"

struct _single_state_x11 s_x11;
struct _single_state_view s_view;
struct _single_state_image s_image;

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

int load_image(char *filename) {
	if (s_image.imlib != NULL) {
		imlib_context_set_image(s_image.imlib);
		imlib_free_image();
	}

	/* TODO error handling */
	Imlib_Load_Error err;
	s_image.imlib = imlib_load_image_with_error_return(filename, &err);

	if (s_image.imlib == NULL) {
		const char *errmsg = imliberr(err);
		fprintf(stderr, "imlib load error: %s\n", errmsg);
	}

	imlib_context_set_image(s_image.imlib);
	s_image.width  = imlib_image_get_width();
	s_image.height = imlib_image_get_height();

	return 0;
}

static void init_imlib() {
	imlib_context_set_display(s_x11.display);

	imlib_context_set_visual(s_x11.visual);
	imlib_context_set_colormap(s_x11.colormap);

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

	s_x11.window = XCreateWindow(
		s_x11.display, DefaultRootWindow(s_x11.display),
		0, 0, 640, 480, 0,
		s_x11.depth,
		InputOutput,
		s_x11.visual,
		CWEventMask,
		&attr);

	/* map/show window */
	XMapWindow(s_x11.display, s_x11.window);

	/* query real size */
	XWindowAttributes xwa;
	XGetWindowAttributes(s_x11.display, s_x11.window, &xwa);
	s_view.win_width  = xwa.width;
	s_view.win_height = xwa.height;
}

void event_handle_x11(Display *dpy) {
	XEvent ev;

	while (XPending(dpy)) {
		XNextEvent(dpy, &ev);

		if (ev.type == ConfigureNotify) {
			s_view.win_width  = ev.xconfigure.width;
			s_view.win_height = ev.xconfigure.height;

			s_view.dirty = 1;
		} else if (ev.type == ButtonPress) {
			input_button_press(ev);
		} else if (ev.type == ButtonRelease) {
			input_button_release(ev);
		} else if (ev.type == MotionNotify) {
			input_pointer_motion(ev);
		}
	}

	XFlush(dpy);
}

void event_loop(Display *dpy, int ctl_fd) {
	int max_fd = (s_x11.fd > ctl_fd ? s_x11.fd : ctl_fd) + 1;

	fd_set fds;
	int ret;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(s_x11.fd, &fds);
		FD_SET(ctl_fd, &fds);

		// fputs("wait for something to happen.\n", stderr);
		ret = select(max_fd, &fds, NULL, NULL, NULL);

		if (ret == -1) {
			perror("select"); /* XXX */
		}

		if (FD_ISSET(ctl_fd, &fds)) {
			// fputs("activity on ctl channel\n", stderr);
			ctl_handle_fd(ctl_fd);
		}

		if (FD_ISSET(s_x11.fd, &fds)) {
			// fputs("activity on X11 connection\n", stderr);
			event_handle_x11(dpy);
		}

		/* redraw the image, if necessary */

		if (s_view.dirty) {
			render_image();
			s_view.dirty = 0;
		}
	}
}

int main(int argc, char *argv[]) {
	/* TODO parse options */
	if (argc != 1) {
		fputs("single does not take arguments. images are loaded via the control channel\n", stderr);
		exit(1);
	}

	/* TODO init imlib */
	s_x11.display  = XOpenDisplay(NULL);
	s_x11.fd       = ConnectionNumber(s_x11.display);
	s_x11.screen   = DefaultScreen(s_x11.display);
	s_x11.gc       = DefaultGC(s_x11.display, s_x11.screen);
	s_x11.colormap = DefaultColormap(s_x11.display, s_x11.screen);
	s_x11.visual   = DefaultVisual(s_x11.display, s_x11.screen);
	s_x11.depth    = DefaultDepth(s_x11.display, s_x11.screen);

	s_view.scale = 1.0;

	init_imlib();

	/* make window */
	make_window();

	XFlush(s_x11.display);

	event_loop(s_x11.display, 0); /* 0 = stdin */
}

