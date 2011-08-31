#include "common/types.h"

int rect_intersect(struct rect r1, struct rect r2) {
	return !(
		r2.topleft.x > r1.topleft.x + r1.dimensions.width  ||
		r2.topleft.x + r2.dimensions.width  < r1.topleft.x ||
		r2.topleft.y > r1.topleft.y + r1.dimensions.height ||
		r2.topleft.y + r2.dimensions.height < r1.topleft.y
	);
}
