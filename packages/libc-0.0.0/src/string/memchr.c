#include <xos/xos.h>
#include <string.h>

void * memchr(const void * s, int c, size_t n)
{
	return xos_memchr(s, c, n);
}
