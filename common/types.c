#include "common/types.h"

/* Scales a rectangle 'obj' so it fits into the rectangle 'fit',
 * preserving the aspect ratio.
 * Smaller rectangles will be upscaled.
 */

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
 * only larger ones will be resized.
 */

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
		r2.tl.x                 > r1.tl.x + r1.dim.width  ||
		r2.tl.x + r2.dim.width  < r1.tl.x                 ||
		r2.tl.y                 > r1.tl.y + r1.dim.height ||
		r2.tl.y + r2.dim.height < r1.tl.y
	);
}

/* does a rectangle include a point? (on border = yes) */

int rect_contains(struct rect r, struct coord c) {
	return (
		r.tl.x <= c.x && r.tl.x + r.dim.width  >= c.x &&
		r.tl.y <= c.y && r.tl.y + r.dim.height >= c.y
	);
}
