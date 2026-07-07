#include <xos/xos.h>
#include <string.h>

void * memset(void * s, int c, size_t n)
{
	return xos_memset(s, c, n);
}
