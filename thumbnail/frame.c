#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Imlib2.h>

#include "common/types.h"

#include "thumbnail/frame.h"

/* API TODO/ideas: extra arg containing config parameters for frame
   content, so that text dimensions can be calculated from the real
   space needed (e.g. from format string) */

static struct coord symbols_frame_size(struct frame *frame) {
	/* TODO calculate real text size instead of hardcoding */
	return COORD(frame->thumb_dim.width, frame->thumb_dim.height + 12);
}

static void symbols_render(struct frame *frame, struct thumbnail *t, struct rect target) {
	/* should it be struct thumbnail?
	   maybe a generic catch-all struct fooprefix_file instead?
	   (but isn't struct thumbnail already that, just oddly named?) */

	/* XXX hm */
	struct coord _rc1 = symbols_frame_size(frame);
	struct coord _rc2 = target.dimensions;
	if (_rc1.x != _rc2.x || _rc1.y != _rc2.y) {
		fprintf(stderr, "[-] target frame size %dx%d does not match configured one %dx%d\n",
			_rc2.x, _rc2.y, _rc1.x, _rc1.y
		);
	}

	if (t->imlib != NULL && !t->failed) {
		struct coord render_dim = coord_scale_to_fit(
			t->thumb_dim,
			frame->thumb_dim
		);

		imlib_blend_image_onto_image(t->imlib, 1,
			/* sxywh */ 0, 0, t->thumb_dim.width, t->thumb_dim.height,
			/* dxywh */
				target.topleft.x + (frame->thumb_dim.width  - render_dim.width ) / 2,
				target.topleft.y + (frame->thumb_dim.height - render_dim.height) / 2,
				render_dim.width, render_dim.height
		);
	}

	/* TODO draw text */

	imlib_add_path_to_font_path("/usr/share/fonts/truetype/ttf-dejavu");
	Imlib_Font font = imlib_load_font("DejaVuSans/10");

	if (font != NULL) {
		imlib_context_set_font(font);
		imlib_context_set_color(255, 255, 255, 255);

		/* text to render */

		char *basename = strrchr(t->filename, '/');
		if (basename == NULL) {
			basename = t->filename;
		} else {
			basename++; /* skip the '/' */
		}

		/* build text. TODO shorten if too long. */

		char *text = malloc(sizeof(char) * (strlen(basename) + 1));

		strcpy(text, basename);

		/* and render it */

		int textwidth, textheight;
		imlib_get_text_size(text, &textwidth, &textheight);

		int tx, ty;
		tx = target.topleft.x + (target.dimensions.width - textwidth) / 2;
		ty = target.topleft.y + frame->thumb_dim.height;

		imlib_text_draw(tx, ty, text);

		free(text);
	}
}

struct frame *frame_new_symbols() {
	struct frame *f = malloc(sizeof(struct frame));

	f->frame_size = symbols_frame_size;
	f->render     = symbols_render;

	return f;
}
