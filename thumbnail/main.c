#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/select.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include <Imlib2.h>

#include "common/input.h"
#include "common/x11.h"

#include "thumbnail/thumbnail.h"
#include "thumbnail/render.h"

Window x11_window;
int win_width, win_height;
int view_dirty = 1;

int scroll_offset = 0;
static int scroll_offset_start;

static void eh_click(struct input_event_click ev) {
	fprintf(stderr, "click event: button %d at %d,%d\n",
		ev.button, ev.x, ev.y);
}

static void eh_drag_start(struct input_event_drag_start ev) {
	fprintf(stderr, "drag start: button %d at %d,%d\n",
		ev.button, ev.start_x, ev.start_y);

	if (ev.button == 1) {
		scroll_offset_start = scroll_offset;
	}
}

static void eh_drag_stop(struct input_event_drag_stop ev) {
	fprintf(stderr, "drag stop: button %d\n",
		ev.button);
}

static void eh_drag_update(struct input_event_drag_update ev) {
	fprintf(stderr, "drag update: button %d now at %d,%d start %d,%d\n",
		ev.button, ev.pointer_x, ev.pointer_y, ev.start_x, ev.start_y);

	if (ev.button == 1) {
		scroll_offset = scroll_offset_start - (ev.pointer_y - ev.start_y);
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

		// fputs("wait for something to happen.\n", stderr);
		ret = select(max_fd, &fds, NULL, NULL, NULL);

		if (ret == -1) {
			perror("select"); /* XXX */
		}

		if (FD_ISSET(x11.fd, &fds)) {
			// fputs("activity on X11 connection\n", stderr);
			event_handle_x11(dpy);
		}

		try_update_thumbnails();

		if (view_dirty) {
			update_view();
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

	event_loop(x11.display, 0); /* 0 = stdin */
}

