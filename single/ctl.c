#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "single/crap.h"

/* this proto parser is fugly. */

void ctl_handle_fd(int fd) {
	/* zomfg hax */

	char command[5];
	memset(command, '\0', 5);
	read(fd, command, 4);

	if (!strcmp(command, "OPEN")) {
		char count;
		read(fd, &count, 1);

		char filename[256];
		read(fd, filename, count);
		filename[count] = '\0';

		spam("load: ");
		spam(filename);
		spam("\n");

		load_image(filename);
		render_image_primitive();
	} else {
		spam("unknown command on pipe: ");
		spam(command);
		spam("\n");
	}
}
