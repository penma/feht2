#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

struct coord {
	union { int x,  width, horizontal; };
	union { int y, height,   vertical; };
};

struct rect {
	struct coord tl;
	struct coord dim;
};

static inline struct coord COORD(int x, int y) {
	return (struct coord){ .x = x, .y = y };
}

static inline struct rect RECT(struct coord tl, struct coord dim) {
	return (struct rect){ .tl = tl, .dim = dim };
}

struct coord coord_scale_to_fit(struct coord obj, struct coord fit);
struct coord coord_downscale_to_fit(struct coord obj, struct coord fit);
int rect_intersect(struct rect r1, struct rect r2);
int rect_contains(struct rect, struct coord);

#endif
