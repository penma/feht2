#include <stdlib.h>
#include <string.h>

#include <Imlib2.h>

#include "common/types.h"

#include "thumbnail/frame.h"

/* API TODO/ideas: extra arg containing config parameters for frame
   content, so that text dimensions can be calculated from the real
   space needed (e.g. from format string) */

/* static */ void symbols_render(struct rect frame, struct thumbnail *t) {
	/* should it be struct thumbnail?
	   maybe a generic catch-all struct fooprefix_file instead? */

	/* XXX here would be a good place to use a struct frame.
	   stores as input requested thumb size and knows it all the time
	   instead of doing this crap calculation.. */
	int th_w = frame.dimensions.width;
	int th_h = frame.dimensions.height - 12;

	if (t->imlib != NULL && !t->failed) {
		struct coord render_dim = coord_scale_to_fit(
			COORD(t->width, t->height),
			COORD(th_w, th_h)
		);

		imlib_blend_image_onto_image(t->imlib, 1,
			/* sxywh */ 0, 0, t->width, t->height,
			/* dxywh */
				frame.topleft.x + (th_w - render_dim.width ) / 2,
				frame.topleft.y + (th_h - render_dim.height) / 2,
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
		tx = frame.topleft.x + (frame.dimensions.width - textwidth) / 2;
		ty = frame.topleft.y + th_h;

		imlib_text_draw(tx, ty, text);

		free(text);
	}
}

/* static */ struct coord symbols_framesize(struct coord thumb_dim) {
	/* TODO calculate real text size instead of hardcoding */
	return COORD(thumb_dim.width, thumb_dim.height + 12);
}

