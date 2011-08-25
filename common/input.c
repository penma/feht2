#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "common/input.h"

/* current pointer state. */

static int button_pressed = 0;
static int moved = 0;
static int drag_start_x,  drag_start_y;
static int drag_last_x,   drag_last_y;
static int drag_warped_x, drag_warped_y;

/* how much movement to ignore, once.
   used when pointer warping. warping the pointer will of course generate
   a pointer motion event, but if we'd handle that, the effect would be
   undone.
   so we store how much we warped, ignore this much movement (and then reset
   the ignore counter). */

static int
	drag_ignore_x = 0,
	drag_ignore_y = 0;

/* limits for pointer movement, from current pointer position.
   will not do pointer warping in some direction if new pointer position
   would exceed these limits. */

static int
	drag_limit_xneg = 0,
	drag_limit_xpos = 0,
	drag_limit_yneg = 0,
	drag_limit_ypos = 0;

/* configured event handlers.
   NULL is allowed and means they're not called. */

static void (*ev_click      )(struct input_event_click)       = NULL;
static void (*ev_hover      )(struct input_event_hover)       = NULL;
static void (*ev_drag_start )(struct input_event_drag_start)  = NULL;
static void (*ev_drag_stop  )(struct input_event_drag_stop)   = NULL;
static void (*ev_drag_update)(struct input_event_drag_update) = NULL;

/* configure event handlers */

void input_set_drag_handlers(
	void (*drag_start )(struct input_event_drag_start),
	void (*drag_stop  )(struct input_event_drag_stop),
	void (*drag_update)(struct input_event_drag_update)
) {
	ev_drag_start  = drag_start;
	ev_drag_stop   = drag_stop;
	ev_drag_update = drag_update;
}

void input_set_click_handler(void (*click)(struct input_event_click)) {
	ev_click = click;
}

void input_set_hover_handler(void (*hover)(struct input_event_hover)) {
	ev_hover = hover;
}

/* XXX */

void input_set_drag_limits(int xneg, int xpos, int yneg, int ypos) {
	drag_limit_xneg = xneg;
	drag_limit_xpos = xpos;
	drag_limit_yneg = yneg;
	drag_limit_ypos = ypos;
}

/* x11 event handling */

#define call_unless_null(_f, ...) do { \
	if (_f != NULL) { \
		_f(__VA_ARGS__); \
	} \
} while (0)

static void xev_press(XEvent ev) {
	button_pressed = ev.xbutton.button;
	moved = 0;

	drag_start_x = drag_last_x = ev.xbutton.x;
	drag_start_y = drag_last_y = ev.xbutton.y;
}

static void xev_release(XEvent ev) {
	if (!moved) {
		call_unless_null(ev_click, (struct input_event_click){
			.button = button_pressed,
			.x      = ev.xbutton.x,
			.y      = ev.xbutton.y,
		});
	} else {
		call_unless_null(ev_drag_stop, (struct input_event_drag_stop){
			.button = button_pressed
		});
	}

	button_pressed = 0;
}

static void xev_motion(XEvent ev) {
	if (button_pressed) {
		if (!moved) {
			moved = 1;
			call_unless_null(ev_drag_start, (struct input_event_drag_start){
				.button = button_pressed,
				.start_x = drag_start_x,
				.start_y = drag_start_y,
			});
		}

		/* pointer warping
		   XXX */

		/* update pointer movement limits.
		   have to do it before calling the handler, because it may
		   update the limits. */
		/* XXX input_set_drag_limits(
			drag_limit_xneg + */

		call_unless_null(ev_drag_update, (struct input_event_drag_update){
			.button = button_pressed,
			.start_x = drag_start_x,
			.start_y = drag_start_y,
			.pointer_x = ev.xmotion.x + drag_warped_x, /* TODO pointer warping */
			.pointer_y = ev.xmotion.y + drag_warped_y, /* dito */
		});
	} else {
		call_unless_null(ev_hover, (struct input_event_hover){
			.x = ev.xmotion.x,
			.y = ev.xmotion.y
		});
	}

	drag_last_x = ev.xmotion.x;
	drag_last_y = ev.xmotion.y;
}

int input_try_xevent(XEvent ev) {
	switch (ev.type) {
		case ButtonPress  : xev_press  (ev); return 1;
		case ButtonRelease: xev_release(ev); return 1;
		case MotionNotify : xev_motion (ev); return 1;
		default           : return 0;
	}
}

