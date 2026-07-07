#include <xos/xos.h>
#include <string.h>

int strcmp(const char * s1, const char * s2)
{
	return xos_strcmp(s1, s2);
}
