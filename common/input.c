#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "common/input.h"

static int button_pressed = 0;
static int moved = 0;
static int drag_start_x, drag_start_y;
static int drag_last_x,  drag_last_y;

void (*input_ev_click      )(struct input_event_click)       = NULL;
void (*input_ev_hover      )(struct input_event_hover)       = NULL;
void (*input_ev_drag_start )(struct input_event_drag_start)  = NULL;
void (*input_ev_drag_stop  )(struct input_event_drag_stop)   = NULL;
void (*input_ev_drag_update)(struct input_event_drag_update) = NULL;

static void xev_press(XEvent ev) {
	button_pressed = ev.xbutton.button;
	moved = 0;

	drag_start_x = drag_last_x = ev.xbutton.x;
	drag_start_y = drag_last_y = ev.xbutton.y;
}

static void xev_release(XEvent ev) {
	if (!moved) {
		if (input_ev_click != NULL) input_ev_click((struct input_event_click){
			.button = button_pressed,
			.x      = ev.xbutton.x,
			.y      = ev.xbutton.y,
		});
	} else {
		if (input_ev_drag_stop != NULL) input_ev_drag_stop((struct input_event_drag_stop){
			.button = button_pressed
		});
	}

	button_pressed = 0;
}

static void xev_motion(XEvent ev) {
	if (button_pressed) {
		if (!moved) {
			moved = 1;
			if (input_ev_drag_start != NULL) input_ev_drag_start((struct input_event_drag_start){
				.button = button_pressed
			});
		}

		if (input_ev_drag_update != NULL) input_ev_drag_update((struct input_event_drag_update){
			.button = button_pressed,
			.start_x = drag_start_x,
			.start_y = drag_start_y,
			.dist_x  = ev.xmotion.x - drag_start_x,
			.dist_y  = ev.xmotion.y - drag_start_y,
			.delta_x = ev.xmotion.x - drag_last_x,
			.delta_y = ev.xmotion.y - drag_last_y
		});
	} else {
		if (input_ev_hover != NULL) input_ev_hover((struct input_event_hover){
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

