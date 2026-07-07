#include <xos/xos.h>
#include <string.h>

char * strncpy(char * dest, const char * src, size_t n)
{
	return xos_strncpy(dest, src, n);
}
