#include <xos/xos.h>
#include <string.h>

int strcasecmp(const char * s1, const char * s2)
{
	return xos_strcasecmp(s1, s2);
}
