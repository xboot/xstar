#include <xos/xos.h>
#include <string.h>

void * memmove(void * dest, const void * src, size_t n)
{
	return xos_memmove(dest, src, n);
}
