#include <errno.h>

static int __libc_errno = 0;

volatile int * __errno_location(void)
{
	return &__libc_errno;
}
