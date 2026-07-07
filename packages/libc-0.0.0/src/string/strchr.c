#include <xos/xos.h>
#include <string.h>

char * strchr(const char * s, int c)
{
	return xos_strchr(s, c);
}
