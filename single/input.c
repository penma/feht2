#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "state.h"

enum pointer_mode {
	MODE_NOTHING,
	MODE_PANNING,
	MODE_ZOOMING
};

static enum pointer_mode pointer_mode = MODE_NOTHING;
static int pointer_last_x, pointer_last_y;

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

void input_button_press(XEvent ev) {
	if (ev.xbutton.button == Button1) {
		pointer_mode = MODE_PANNING;
	} else if (ev.xbutton.button == Button2) {
		pointer_mode = MODE_ZOOMING;
	}

	pointer_last_x = ev.xbutton.x;
	pointer_last_y = ev.xbutton.y;
}

void input_button_release(XEvent ev) {
	pointer_mode = MODE_NOTHING;

	view_sanitize_offsets();
	s_view.dirty = 1;
}

void input_pointer_motion(XEvent ev) {
	if (pointer_mode == MODE_PANNING) {
		int dx = ev.xbutton.x - pointer_last_x;
		int dy = ev.xbutton.y - pointer_last_y;

		s_view.pan_x += dx;
		s_view.pan_y += dy;

		view_sanitize_offsets();

		s_view.dirty = 1;
	} else if (pointer_mode == MODE_ZOOMING) {
		s_view.scale = ev.xbutton.x / 100.0;

		s_view.dirty = 1;
	}

	pointer_last_x = ev.xbutton.x;
	pointer_last_y = ev.xbutton.y;
}
