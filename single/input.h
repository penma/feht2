#ifndef _SINGLE_INPUT_H
#define _SINGLE_INPUT_H

#include <X11/Xlib.h>

void input_button_press(XEvent);
void input_button_release(XEvent);
void input_pointer_motion(XEvent);

#endif
