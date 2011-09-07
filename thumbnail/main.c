#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "common/input.h"
#include "common/x11.h"
#include "common/surface.h"

#include "thumbnail/frame.h"
#include "thumbnail/layout.h"
#include "thumbnail/view.h"
#include "thumbnail/thumbnail.h"

struct x11_connection *x11;
struct view *view;

static int must_update = 0;

int zooming = 0;

static int scroll_offset_start;

static struct coord thumb_dim_start;

static void eh_click(int button, struct coord pos) {
	fprintf(stderr, "click event: button %d at %d,%d\n",
		button, pos.x, pos.y);

	int n = layout_frame_number_by_coord(view->layout, COORD(pos.x, pos.y + view->scroll_offset));
	if (n == -1) {
		fputs("... no image there.\n", stderr);
	} else {
		fprintf(stderr, "clicked at image %d (filename = %s)\n",
			n,
			thumbnails[n]->filename);

		/* show image large (hack) */

		if (!fork()) {
			if (!fork()) {
				char **varg = malloc(sizeof(char*) * (view->layout->frame_count + 4));
				int va = 3;
				struct thumbnail **p = thumbnails;
				while (*p != NULL) {
					struct stat s;
					stat((*p)->filename, &s);
					if (S_ISREG(s.st_mode)) {
						varg[va] = (*p)->filename;
						va++;
					}
					p++;
				}
				varg[0] = "feh";
				varg[1] = "--start-at";
				varg[2] = thumbnails[n]->filename;
				varg[va] = NULL;

				execvp("feh", varg);
				perror("exec");
				exit(1);
			}
			exit(0);
		}
	}
}

static void eh_drag_start(int button, struct coord start) {
	fprintf(stderr, "drag start: button %d at %d,%d\n",
		button, start.x, start.y);

	if (button == 1) {
		scroll_offset_start = view->scroll_offset;
	} else if (button == 2) {
		zooming = 1;
		thumb_dim_start = view->frame->thumb_dim;
	}
}

static void eh_drag_stop(int button) {
	fprintf(stderr, "drag stop: button %d\n",
		button);

	if (button == 2) {
		zooming = 0;
	}
}

static void eh_drag_update(int button, struct coord start, struct coord pointer) {
	fprintf(stderr, "drag update: button %d now at %d,%d start %d,%d\n",
		button, pointer.x, pointer.y, start.x, start.y);

	if (button == 1) {
		view->scroll_offset = scroll_offset_start - (pointer.y - start.y);
		view->dirty = 1; /* XXX should be generated or even handled by view itself */
	} else if (button == 2) {
		view->frame->thumb_dim = COORD(
			thumb_dim_start.width  + (pointer.x - start.x),
			thumb_dim_start.height + (pointer.y - start.y)
		);
		view->layout->frame = view->frame->frame_size(view->frame);
		layout_recompute(view->layout);
		fprintf(stderr, "thumb size now %dx%d\n",
			view->frame->thumb_dim.width, view->frame->thumb_dim.height);
		view->dirty = 1; /* XXX should be generated from inside view */
	}
}

static void event_handle_x11(Display *dpy) {
	XEvent ev;

	while (XPending(dpy)) {
		XNextEvent(dpy, &ev);

		if (input_try_xevent(ev)) {
			continue;
		}

		if (ev.type == ConfigureNotify) {
			view->layout->window = COORD(
				ev.xconfigure.width,
				ev.xconfigure.height
			);
			layout_recompute(view->layout);

			view->dirty = 1;
		if (ev.type == Expose) {
			view->dirty = 1;
		} else {
			const char *event_names[] = { "", "",
				"KeyPress"        ,"KeyRelease"      ,"ButtonPress"    ,"ButtonRelease",  /* 02..05 */
				"MotionNotify"    ,"EnterNotify"     ,"LeaveNotify"    ,"FocusIn",        /* 06..09 */
				"FocusOut"        ,"KeymapNotify"    ,"Expose"         ,"GraphicsExpose", /* 10..13 */
				"NoExpose"        ,"VisibilityNotify","CreateNotify"   ,"DestroyNotify",  /* 14..17 */
				"UnmapNotify"     ,"MapNotify"       ,"MapRequest"     ,"ReparentNotify", /* 18..21 */
				"ConfigureNotify" ,"ConfigureRequest","GravityNotify"  ,"ResizeRequest",  /* 22..25 */
				"CirculateNotify" ,"CirculateRequest","PropertyNotify" ,"SelectionClear", /* 26..29 */
				"SelectionRequest","SelectionNotify" ,"ColormapNotify" ,"ClientMessage",  /* 30..33 */
				"MappingNotify"   ,"GenericEvent"                                         /* 34..35 */
			};

			fprintf(stderr, "[*] unhandled X event type %d (%s)\n",
				ev.type,
				((ev.type >= 2 && ev.type <= 35) ? event_names[ev.type] : "?")
			);
		}
	}

	XFlush(dpy);
}

static void event_loop(Display *dpy, int ctl_fd) {
	int max_fd = x11->fd + 1;

	fd_set fds;
	int ret;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(x11->fd, &fds);

		ret = select(
			max_fd, &fds, NULL, NULL,
			must_update ? &(struct timeval){ 0, 0 } : NULL
		);

		if (ret == -1) {
			perror("select"); /* XXX */
		}

		/* sometimes, especially during thumbnail updating, we do not
		 * see activity on the FD even though there are events
		 * available. I think this happens after a redraw followed by
		 * XFlush, which makes Xlib poll the FD for events. So by the
		 * time we call select here, they're already read.
		 * We can use XPending to find out about that. When not
		 * updating thumbnails, new events still trigger the FD select,
		 * so we use that to wake up.
		 */

		if (FD_ISSET(x11->fd, &fds) || XPending(dpy)) {
			event_handle_x11(dpy);
		}

		must_update = try_update_thumbnails();

		/* FIXME: make sure that really every update causes this to
		 * be set.
		 */

		if (must_update) {
			view->dirty = 1;
		}

		if (view->dirty) {
			view_render(view);
		}

		XFlush(dpy);
	}
}

int main(int argc, char *argv[]) {
	/* TODO parse options */
	if (argc != 2) {
		fputs("thumbnail must be invoked with a single argument specifying the directory to load\n", stderr);
		exit(1);
	}

	x11 = x11_connect();
	x11_setup_imlib(x11);

	/* setup input foo */

	input_set_click_handler(eh_click);
	input_set_drag_handlers(eh_drag_start, eh_drag_stop, eh_drag_update);

	/* make window */

	view = view_new();
	view->surface = surface_new();
	view->surface->x11 = x11;
	view->surface->window = x11_make_window(x11);

	XStoreName(x11->display, view->surface->window, argv[1]);
	XFlush(x11->display);

	/* blafoo */

	struct dirent **dir_entries;
	int dir_count = scandir(argv[1], &dir_entries, NULL, alphasort);

	thumbnails = malloc(sizeof(struct thumbnail *) * (dir_count + 1));

	for (int i = 0; i < dir_count; i++) {
		thumbnails[i] = malloc(sizeof(struct thumbnail));
		struct thumbnail *t = thumbnails[i];

		char *fn = dir_entries[i]->d_name;
		asprintf(&t->filename, "%s/%s", argv[1], fn);
		t->failed = 0;
		t->imlib = NULL;

		free(dir_entries[i]);
	}

	free(dir_entries);

	thumbnails[dir_count] = NULL;

	/* construct layout and stuff */

	view->frame = frame_new_symbols();
	view->frame->thumb_dim = COORD(200, 150);

	view->layout = layout_new();
	view->layout->window  = COORD(0, 0);
	view->layout->spacing = COORD(20, 20);
	view->layout->frame   = view->frame->frame_size(view->frame);
	view->layout->frame_count = dir_count;
	layout_recompute(view->layout);

	must_update = 1;

	event_loop(x11->display, 0); /* 0 = stdin */
}

