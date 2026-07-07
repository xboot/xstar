#include <xos/xos.h>
#include <string.h>

size_t strlcat(char * dest, const char * src, size_t n)
{
	return xos_strlcat(dest, src, n);
}
