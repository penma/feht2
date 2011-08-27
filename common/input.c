#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "common/input.h"

/* current pointer state. */

static int button_pressed = 0;
static int moved = 0;
static struct coord drag_start;
static struct coord drag_last;
static struct coord drag_warped;

/* how much movement to ignore, once.
   used when pointer warping. warping the pointer will of course generate
   a pointer motion event, but if we'd handle that, the effect would be
   undone.
   so we store how much we warped, ignore this much movement (and then reset
   the ignore counter). */

static struct coord drag_ignore = { .x = 0, .y = 0 };

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

static void (*ev_click      )(int, struct coord)               = NULL;
static void (*ev_hover      )(     struct coord)               = NULL;
static void (*ev_drag_start )(int, struct coord)               = NULL;
static void (*ev_drag_stop  )(int)                             = NULL;
static void (*ev_drag_update)(int, struct coord, struct coord) = NULL;

/* configure event handlers */

void input_set_drag_handlers(
	void (*drag_start )(int, struct coord),
	void (*drag_stop  )(int),
	void (*drag_update)(int, struct coord, struct coord)
) {
	ev_drag_start  = drag_start;
	ev_drag_stop   = drag_stop;
	ev_drag_update = drag_update;
}

void input_set_click_handler(void (*click)(int, struct coord)) {
	ev_click = click;
}

void input_set_hover_handler(void (*hover)(struct coord)) {
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
	/* we only handle one drag at a time, so if another button was pressed,
	   emit a stop event for that. */

	if (button_pressed != 0 && moved) {
		call_unless_null(ev_drag_stop,
			button_pressed
		);
	}

	/* store state only.
	   we do not yet know if this is a drag or a click. */

	button_pressed = ev.xbutton.button;
	moved = 0;

	drag_start.x = drag_last.x = ev.xbutton.x;
	drag_start.y = drag_last.y = ev.xbutton.y;
}

static void xev_release(XEvent ev) {
	/* don't emit a release event with no button.
	   this might happen after pressing multiple buttons at once,
	   which is not planned to be handled. */

	if (button_pressed == 0) {
		return;
	}

	if (moved) {
		call_unless_null(ev_drag_stop,
			button_pressed
		);
	} else {
		call_unless_null(ev_click,
			button_pressed,
			(struct coord){ .x = ev.xbutton.x, .y = ev.xbutton.y }
		);
	}

	button_pressed = 0;
}

static void xev_motion(XEvent ev) {
	if (button_pressed == 0) {
		call_unless_null(ev_hover,
			(struct coord){ .x = ev.xmotion.x, .y = ev.xmotion.y }
		);

		return;
	}

	if (!moved) {
		moved = 1;
		call_unless_null(ev_drag_start,
			button_pressed,
			drag_start
		);
	}

	/* pointer warping
	   XXX */

	/* update pointer movement limits.
	   have to do it before calling the handler, because it may
	   update the limits. */
	/* XXX input_set_drag_limits(
		drag_limit_xneg + */

	call_unless_null(ev_drag_update,
		button_pressed,
		drag_start,
		(struct coord) {
			.x = ev.xmotion.x + drag_warped.x, /* TODO pointer warping */
			.y = ev.xmotion.y + drag_warped.y, /* dito */
		}
	);

	drag_last.x = ev.xmotion.x;
	drag_last.y = ev.xmotion.y;
}

int input_try_xevent(XEvent ev) {
	switch (ev.type) {
		case ButtonPress  : xev_press  (ev); return 1;
		case ButtonRelease: xev_release(ev); return 1;
		case MotionNotify : xev_motion (ev); return 1;
		default           : return 0;
	}
}

