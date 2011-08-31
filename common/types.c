#include "common/types.h"

/* Scales a rectangle 'obj' so it fits into the rectangle 'fit',
   preserving the aspect ratio.
   Smaller rectangles will be upscaled. */

struct coord coord_scale_to_fit(struct coord obj, struct coord fit) {
	struct coord new;

	new.x = fit.x;
	new.y = fit.x * (double)obj.y / obj.x;

	if (new.y > fit.y) {
		new.x = fit.y * (double)obj.x / obj.y;
		new.y = fit.y;
	}

	return new;
}

/* Like coord_scale_to_fit, but smaller rectangles retain their size,
   only larger ones will be resized. */

struct coord coord_downscale_to_fit(struct coord obj, struct coord fit) {
	if (obj.x < fit.x && obj.y < fit.y) {
		return obj;
	} else {
		return coord_scale_to_fit(obj, fit);
	}
}

/* do two rectangles intersect? */

int rect_intersect(struct rect r1, struct rect r2) {
	return !(
		r2.topleft.x > r1.topleft.x + r1.dimensions.width  ||
		r2.topleft.x + r2.dimensions.width  < r1.topleft.x ||
		r2.topleft.y > r1.topleft.y + r1.dimensions.height ||
		r2.topleft.y + r2.dimensions.height < r1.topleft.y
	);
}
