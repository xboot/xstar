#include <xos/xos.h>
#include <string.h>

char * strnstr(const char * s1, const char * s2, size_t n)
{
	return xos_strnstr(s1, s2, n);
}
