#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

#include <Imlib2.h>

#include "single/ctl.h"
#include "single/image.h"
#include "single/state.h"

struct _single_state_x11 s_x11;
struct _single_state_view s_view;
struct _single_state_input s_input;
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

static int min(int a, int b) {
	return a < b ? a : b;
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

static void render_background() {
	static Pixmap checks_pmap = None;
	Imlib_Image checks = NULL;

	if (checks_pmap == None) {
		checks = imlib_create_image(16, 16);

		if (!checks) {
			fprintf(stderr, "Unable to create a teeny weeny imlib image. I detect problems\n");
		}

		imlib_context_set_image(checks);

		imlib_context_set_color(144, 144, 144, 255);
		imlib_image_fill_rectangle(0, 0, 16, 16);

		imlib_context_set_color(100, 100, 100, 255);
		imlib_image_fill_rectangle(0, 0, 8, 8);
		imlib_image_fill_rectangle(8, 8, 8, 8);

		checks_pmap = XCreatePixmap(s_x11.display, s_x11.window, 16, 16, 24);

		imlib_context_set_drawable(checks_pmap);
		imlib_render_image_on_drawable(0, 0);
		imlib_free_image_and_decache();
	}

	static GC gc = None;
	XGCValues gcval;

	if (gc == None) {
		gcval.tile = checks_pmap;
		gcval.fill_style = FillTiled;
		gc = XCreateGC(s_x11.display, s_x11.window, GCTile | GCFillStyle, &gcval);
	}

	XFillRectangle(s_x11.display, s_x11.pixmap, gc, 0, 0, s_view.win_width, s_view.win_height);
}

void render_image_primitive() {
	fputs("rendering\n", stderr);

	/* render the background */
	render_background();

	/* now render the actual image */
	if (s_image.imlib != NULL) {
		/* calculate draw offsets. */
		int sx, sy, dx, dy, sw, sh, dw, dh;

		if (s_view.pan_x < 0) {
			/* left part of image is hidden */
			sx = -s_view.pan_x;
			dx = 0;
		} else {
			/* left part of image is somewhere in the window */
			sx = 0;
			dx = s_view.pan_x;
		}

		if (s_view.pan_y < 0) {
			/* top part of image is hidden */
			sy = -s_view.pan_y;
			dy = 0;
		} else {
			/* top part of image is somewhere in the window */
			sy = 0;
			dy = s_view.pan_y;
		}

		/* how much of the image is visible? */
		dw = min(s_view.win_width  - dx, s_image.width  - sx);
		dh = min(s_view.win_height - dy, s_image.height - sy);
		sw = dw; sh = dh;

		fprintf(stderr, "now rendering to coordinates s=%d,%d/%dx%d d=%d,%d/%dx%d\n", sx, sy, sw, sh, dx, dy, dw, dh);

		imlib_context_set_image(s_image.imlib);
		imlib_context_set_drawable(s_x11.pixmap);
		imlib_render_image_part_on_drawable_at_size(
			/* sxywh */ sx, sy, sw, sh,
			/* dxywh */ dx, dy, dw, dh
		);
	} else {
		fputs("not rendering because there is no image.\n", stderr);
	}

	XSetWindowBackgroundPixmap(s_x11.display, s_x11.window, s_x11.pixmap);
	XClearWindow(s_x11.display, s_x11.window);

	XFlush(s_x11.display);
}

static void create_view_pixmap() {
	if (s_x11.pixmap != None) {
		XFreePixmap(s_x11.display, s_x11.pixmap);
	}

	s_x11.pixmap = XCreatePixmap(s_x11.display, s_x11.window, s_view.win_width, s_view.win_height, 24);
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

	/* query size */
	XWindowAttributes xwa;
	XGetWindowAttributes(s_x11.display, s_x11.window, &xwa);
	s_view.win_width  = xwa.width;
	s_view.win_height = xwa.height;
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

		if (ev.type == ConfigureNotify) {
			s_view.win_width  = ev.xconfigure.width;
			s_view.win_height = ev.xconfigure.height;

			create_view_pixmap();
			render_image_primitive();
		} else if (ev.type == ButtonPress) {
			if (ev.xbutton.button == Button1) {
				s_input.pointer_mode = POINTER_MODE_PANNING;
				s_input.pointer_last_x = ev.xbutton.x;
				s_input.pointer_last_y = ev.xbutton.y;
			} else if (ev.xbutton.button == Button2) {
				s_input.pointer_mode = POINTER_MODE_ZOOMING;
				s_input.pointer_last_x = ev.xbutton.x;
				s_input.pointer_last_y = ev.xbutton.y;
			}
		} else if (ev.type == ButtonRelease) {
			s_input.pointer_mode = POINTER_MODE_NOTHING;
		} else if (ev.type == MotionNotify) {
			if (s_input.pointer_mode == POINTER_MODE_PANNING) {
				int dx = ev.xbutton.x - s_input.pointer_last_x;
				int dy = ev.xbutton.y - s_input.pointer_last_y;

				s_view.pan_x += dx;
				s_view.pan_y += dy;

				s_input.pointer_last_x += dx;
				s_input.pointer_last_y += dy;

				render_image_primitive();
			} else if (s_input.pointer_mode == POINTER_MODE_ZOOMING) {
				s_view.scale = ev.xbutton.x / 100.0;

				s_input.pointer_last_x = ev.xbutton.x;
				s_input.pointer_last_y = ev.xbutton.y;

				render_image_primitive();
			}
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

		fputs("wait for something to happen.\n", stderr);
		ret = select(max_fd, &fds, NULL, NULL, NULL);

		if (ret == -1) {
			perror("select"); /* XXX */
		}

		if (FD_ISSET(ctl_fd, &fds)) {
			fputs("activity on ctl channel\n", stderr);
			ctl_handle_fd(ctl_fd);
		}

		if (FD_ISSET(s_x11.fd, &fds)) {
			fputs("activity on X11 connection\n", stderr);
			event_handle_x11(dpy);
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

	s_x11.pixmap   = None;

	s_input.pointer_mode = POINTER_MODE_NOTHING;

	init_imlib();

	/* make window */
	make_window();
	create_view_pixmap();

	XFlush(s_x11.display);

	event_loop(s_x11.display, 0); /* 0 = stdin */
}

