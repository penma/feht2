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
}

void input_pointer_motion(XEvent ev) {
	if (pointer_mode == MODE_PANNING) {
		int dx = ev.xbutton.x - pointer_last_x;
		int dy = ev.xbutton.y - pointer_last_y;

		s_view.pan_x += dx;
		s_view.pan_y += dy;

		s_view.dirty = 1;
	} else if (pointer_mode == MODE_ZOOMING) {
		s_view.scale = ev.xbutton.x / 100.0;

		s_view.dirty = 1;
	}

	pointer_last_x = ev.xbutton.x;
	pointer_last_y = ev.xbutton.y;
}
