#include <xos/xos.h>
#include <string.h>

size_t strlen(const char * s)
{
	return xos_strlen(s);
}
