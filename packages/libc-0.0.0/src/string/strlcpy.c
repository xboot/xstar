#include <xos/xos.h>
#include <string.h>

size_t strlcpy(char * dest, const char * src, size_t n)
{
	return xos_strlcpy(dest, src, n);
}
