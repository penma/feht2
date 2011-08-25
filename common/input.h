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
	int start_x, start_y;
};

struct input_event_drag_stop {
	int button;
};

struct input_event_drag_update {
	int button;
	int   start_x,   start_y; /* pointer position at drag start */
	int pointer_x, pointer_y; /* virtual pointer position now */
};

void input_set_drag_handlers(
	void (*drag_start )(struct input_event_drag_start),
	void (*drag_stop  )(struct input_event_drag_stop),
	void (*drag_update)(struct input_event_drag_update)
);

void input_set_click_handler(void (*click)(struct input_event_click));

void input_set_hover_handler(void (*hover)(struct input_event_hover));

void input_set_drag_limits(int xneg, int xpos, int yneg, int ypos);

int input_try_xevent(XEvent);

#endif
