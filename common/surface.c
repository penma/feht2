#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <Imlib2.h>

#include "common/surface.h"

struct surface *surface_new() {
	struct surface *s = malloc(sizeof(struct surface));

	s->x11 = NULL;
	s->window = None;
	s->imlib = NULL;

	return s;
}

void surface_ensure_imlib(struct surface *s) {
	/* TODO: this function originally only made a new image if the
	 * size did not change. Find out if it gives any speed advantage
	 * to do so (wouldn't be surprised if not, it's Imlib...).
	 * Also, find out if the XGetWindowAttributes call is necessary.
	 */

	if (s->window == None) {
		fputs("[-] no window present, can't create imlib surface\n",
			stderr);
		return;
	}

	if (s->imlib != NULL) {
		imlib_context_set_image(s->imlib);
		imlib_free_image_and_decache();
	}

	XWindowAttributes xwa;
	XGetWindowAttributes(s->x11->display, s->window, &xwa);

	if (xwa.width == 0)
		xwa.width = 1;
	if (xwa.height == 0)
		xwa.height = 1;

	s->imlib = imlib_create_image(xwa.width, xwa.height);
}

void surface_transfer(struct surface *s) {
	imlib_context_set_image(s->imlib);
	imlib_context_set_drawable(s->window);
	imlib_render_image_on_drawable(0, 0);
}

