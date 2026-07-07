#include <xos/xos.h>
#include <string.h>

int strnicmp(const char * s1, const char * s2, size_t n)
{
	return xos_strncasecmp(s1, s2, n);
}
