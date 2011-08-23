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

extern void (*input_ev_click      )(struct input_event_click);
extern void (*input_ev_hover      )(struct input_event_hover);
extern void (*input_ev_drag_start )(struct input_event_drag_start);
extern void (*input_ev_drag_stop  )(struct input_event_drag_stop);
extern void (*input_ev_drag_update)(struct input_event_drag_update);

int input_try_xevent(XEvent);

#endif
