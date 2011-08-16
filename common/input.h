#ifndef _COMMON_INPUT_H
#define _COMMON_INPUT_H

#include <X11/Xlib.h>

struct input_event_click {
	int button;
	int x, y;
};

struct input_event_hover {
	int x, y;
};

struct input_event_drag_start {
	int button;
};

struct input_event_drag_stop {
	int button;
};

struct input_event_drag_update {
	int button;
	int start_x, start_y; /* pointer position at drag start */
	int  dist_x,  dist_y; /* total distance since drag start */
	int delta_x, delta_y; /* delta since last drag event */
};

int input_try_xevent(XEvent);
void input_set_handler_click(void (*f)(struct input_event_click));
void input_set_handler_hover(void (*f)(struct input_event_hover));
void input_set_handler_drag_start(void (*f)(struct input_event_drag_start));
void input_set_handler_drag_stop(void (*f)(struct input_event_drag_stop));
void input_set_handler_drag_update(void (*f)(struct input_event_drag_update));

#endif
