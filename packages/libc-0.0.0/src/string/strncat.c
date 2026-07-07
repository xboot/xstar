#include <xos/xos.h>
#include <string.h>

char * strncat(char * dest, const char * src, size_t n)
{
	return xos_strncat(dest, src, n);
}
