#ifndef _COMMON_INPUT_H
#define _COMMON_INPUT_H

#include <X11/Xlib.h>

#include "common/types.h"

void input_set_drag_handlers(
	void (*drag_start )(int, struct coord),
	void (*drag_stop  )(int),
	void (*drag_update)(int, struct coord, struct coord)
);

void input_set_click_handler(void (*click)(int, struct coord));
void input_set_hover_handler(void (*hover)(     struct coord));

void input_set_drag_limits(int xneg, int xpos, int yneg, int ypos);

int input_try_xevent(XEvent);

#endif
