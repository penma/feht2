#include <string.h>
#include <unistd.h>

void spam(const char *foo) {
	write(2, foo, strlen(foo));
}
