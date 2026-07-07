#include <xos/xos.h>
#include <string.h>

size_t strnlen(const char * s, size_t n)
{
	return xos_strnlen(s, n);
}
