#include <xos/xos.h>
#include <string.h>

int memcmp(const void * s1, const void * s2, size_t n)
{
	return xos_memcmp(s1, s2, n);
}
