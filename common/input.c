#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "common/input.h"

static int button_pressed = 0;
static int moved = 0;
static int drag_start_x, drag_start_y;
static int drag_last_x,  drag_last_y;

static void (*event_click      )(struct input_event_click)       = NULL;
static void (*event_hover      )(struct input_event_hover)       = NULL;
static void (*event_drag_start )(struct input_event_drag_start)  = NULL;
static void (*event_drag_stop  )(struct input_event_drag_stop)   = NULL;
static void (*event_drag_update)(struct input_event_drag_update) = NULL;

static void xev_press(XEvent ev) {
	button_pressed = ev.xbutton.button;
	moved = 0;

	drag_start_x = drag_last_x = ev.xbutton.x;
	drag_start_y = drag_last_y = ev.xbutton.y;
}

static void xev_release(XEvent ev) {
	if (!moved) {
		if (event_click != NULL) event_click((struct input_event_click){
			.button = button_pressed,
			.x      = ev.xbutton.x,
			.y      = ev.xbutton.y,
		});
	} else {
		if (event_drag_stop != NULL) event_drag_stop((struct input_event_drag_stop){
			.button = button_pressed
		});
	}

	button_pressed = 0;
}

static void xev_motion(XEvent ev) {
	if (button_pressed) {
		if (!moved) {
			moved = 1;
			if (event_drag_start != NULL) event_drag_start((struct input_event_drag_start){
				.button = button_pressed
			});
		}

		if (event_drag_update != NULL) event_drag_update((struct input_event_drag_update){
			.button = button_pressed,
			.start_x = drag_start_x,
			.start_y = drag_start_y,
			.dist_x  = ev.xmotion.x - drag_start_x,
			.dist_y  = ev.xmotion.y - drag_start_y,
			.delta_x = ev.xmotion.x - drag_last_x,
			.delta_y = ev.xmotion.y - drag_last_y
		});
	} else {
		if (event_hover != NULL) event_hover((struct input_event_hover){
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

void input_set_handler_click(void (*f)(struct input_event_click)) {
	event_click = f;
}

void input_set_handler_hover(void (*f)(struct input_event_hover)) {
	event_hover = f;
}

void input_set_handler_drag_start(void (*f)(struct input_event_drag_start)) {
	event_drag_start = f;
}

void input_set_handler_drag_stop(void (*f)(struct input_event_drag_stop)) {
	event_drag_stop = f;
}

void input_set_handler_drag_update(void (*f)(struct input_event_drag_update)) {
	event_drag_update = f;
}
