#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "common/input.h"
#include "single/state.h"

enum pointer_mode {
	MODE_NOTHING,
	MODE_PAN,
	MODE_ZOOM
};

static enum pointer_mode pointer_mode = MODE_NOTHING;

static int min(int a, int b) {
	return a < b ? a : b;
}

static int max(int a, int b) {
	return a > b ? a : b;
}

void view_sanitize_offsets() {
	/* ensure that the image is not outside the viewport when it is
	   possible to completely display there.
	   this prevents accidentally moving the image into nowhere
	   and also hides funny bugs in the renderer. */

	int far_left, far_top;
	int min_x, max_x, max_y, min_y;

	far_left = s_view.win_width  - (s_image.width  * s_view.scale);
	far_top  = s_view.win_height - (s_image.height * s_view.scale);

	min_x = min(far_left, 0);
	max_x = max(far_left, 0);
	min_y = min(far_top, 0);
	max_y = max(far_top, 0);

	s_view.pan_x = max(min_x, min(max_x, s_view.pan_x));
	s_view.pan_y = max(min_y, min(max_y, s_view.pan_y));
}

static void eh_drag_start(struct input_event_drag_start e) {
	if (e.button == Button1) {
		fputs("start panning\n", stderr);
		pointer_mode = MODE_PAN;
	} else if (e.button == Button2) {
		fputs("start zooming\n", stderr);
		pointer_mode = MODE_ZOOM;
	}
}

static void eh_drag_stop(struct input_event_drag_stop e) {
	fputs("stop panning or zooming\n", stderr);
	pointer_mode = MODE_NOTHING;

	view_sanitize_offsets();

	s_view.dirty = 1;
}

static void eh_drag_update(struct input_event_drag_update e) {
	fprintf(stderr, "drag update: start=%d,%d dist=%d,%d delta=%d,%d button=%d\n",
		e.start_x, e.start_y,
		e.dist_x, e.dist_y,
		e.delta_x, e.delta_y,
		e.button
	);

	if (pointer_mode == MODE_PAN) {
		s_view.pan_x += e.delta_x;
		s_view.pan_y += e.delta_y;

		view_sanitize_offsets();

		s_view.dirty = 1;
	} else if (pointer_mode == MODE_ZOOM) {
		s_view.scale = (e.start_x + e.dist_x) / 100.0;

		s_view.dirty = 1;
	}
}

static void eh_click(struct input_event_click e) {
	fprintf(stderr, "click: %d,%d button=%d\n", e.x, e.y, e.button);
}

void input_init() {
	input_set_handler_click(eh_click);
	input_set_handler_drag_start(eh_drag_start);
	input_set_handler_drag_stop(eh_drag_stop);
	input_set_handler_drag_update(eh_drag_update);
}
