#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

struct coord {
	union { int x,  width, horizontal; };
	union { int y, height,   vertical; };
};

static inline struct coord coord(int x, int y) {
	return (struct coord){ .x = x, .y = y };
}

#endif
