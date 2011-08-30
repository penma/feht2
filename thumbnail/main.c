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

#include "thumbnail/layout.h"
#include "thumbnail/thumbnail.h"
#include "thumbnail/render.h"

extern int thumb_width, thumb_height;

Window x11_window;
int win_width, win_height;
int view_dirty = 1;

static int must_update = 0;

int scroll_offset = 0;
static int scroll_offset_start;

static int thumb_w_start, thumb_h_start;

static void eh_click(int button, struct coord pos) {
	fprintf(stderr, "click event: button %d at %d,%d\n",
		button, pos.x, pos.y);

	/* XXX construct layout elsewhere */
	struct layout *L = layout_new();
	L->window  = COORD(win_width, win_height);
	L->spacing = COORD(20, 20);
	L->frame   = COORD(thumb_width, thumb_height + 12); /* text height, XXX */

	{
		L->frame_count = 0;
		struct thumbnail **p = thumbnails;
		while (*p != NULL) {
			p++;
			L->frame_count++;
		}
	}

	layout_recompute(L);

	int n = layout_frame_number_by_coordinate(L, COORD(pos.x, pos.y + scroll_offset));
	if (n == -1) {
		fputs("... no image there.\n", stderr);
	} else {
		fprintf(stderr, "clicked at image %d (filename = %s)\n",
			n,
			thumbnails[n]->filename);

		/* show image large (hack) */

		if (!fork()) {
			if (!fork()) {
				char **varg = malloc(sizeof(char*) * (L->frame_count + 4));
				int va = 3;
				int start_at = n;
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
		scroll_offset_start = scroll_offset;
	} else if (button == 2) {
		thumb_w_start = thumb_width;
		thumb_h_start = thumb_height;
	}
}

static void eh_drag_stop(int button) {
	fprintf(stderr, "drag stop: button %d\n",
		button);
}

static void eh_drag_update(int button, struct coord start, struct coord pointer) {
	fprintf(stderr, "drag update: button %d now at %d,%d start %d,%d\n",
		button, pointer.x, pointer.y, start.x, start.y);

	if (button == 1) {
		scroll_offset = scroll_offset_start - (pointer.y - start.y);
		view_dirty = 1;
	} else if (button == 2) {
		thumb_width  = thumb_w_start + (pointer.x - start.x);
		thumb_height = thumb_h_start + (pointer.y - start.y);
		view_dirty = 1;
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
			win_width  = ev.xconfigure.width;
			win_height = ev.xconfigure.height;

			view_dirty = 1;
		} else if (ev.type == Expose) {
			view_dirty = 1;
		} else {
			fprintf(stderr, "unknown X event type %d\n", ev.type);
		}
	}

	XFlush(dpy);
}

static void event_loop(Display *dpy, int ctl_fd) {
	int max_fd = x11.fd + 1;

	fd_set fds;
	int ret;

	while (1) {
		FD_ZERO(&fds);
		FD_SET(x11.fd, &fds);

		ret = select(max_fd, &fds, NULL, NULL, must_update ? &(struct timeval){ 0, 0 } : NULL);

		if (ret == -1) {
			perror("select"); /* XXX */
		}

		/* sometimes, especially during thumbnail updating, we do not
		   see activity on the FD even though there are events available.
		   I think this happens after a redraw followed by XFlush, which
		   makes Xlib poll the FD for events. So by the time we call
		   select here, they're already read.
		   We can use XPending to find out about that. When not updating
		   thumbnails, new events still trigger the FD select, so we use
		   that to wake up. */

		if (FD_ISSET(x11.fd, &fds) || XPending(dpy)) {
			event_handle_x11(dpy);
		}

		must_update = try_update_thumbnails();
		if (must_update) { /* HACK. but if it's 1, something was indeed updated */
			view_dirty = 1;
		}

		if (view_dirty) {
			update_view();
			view_dirty = 0;
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

	setup_x11();
	setup_imlib();

	/* setup input foo */

	input_set_click_handler(eh_click);
	input_set_drag_handlers(eh_drag_start, eh_drag_stop, eh_drag_update);

	/* make window */
	make_window();

	XFlush(x11.display);
	XStoreName(x11.display, x11_window, argv[1]);

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

	must_update = 1;

	event_loop(x11.display, 0); /* 0 = stdin */
}

