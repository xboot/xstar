#include <xos/xos.h>
#include <string.h>

int strncmp(const char * s1, const char * s2, size_t n)
{
	return xos_strncmp(s1, s2, n);
}
