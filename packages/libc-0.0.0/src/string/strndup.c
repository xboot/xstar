#include <xos/xos.h>
#include <string.h>

char * strndup(const char * s, size_t n)
{
	return xos_strndup(s, n);
}
