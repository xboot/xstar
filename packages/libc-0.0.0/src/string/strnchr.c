#include <xos/xos.h>
#include <string.h>

char * strnchr(const char * s, size_t n, int c)
{
	return xos_strnchr(s, n, c);
}
