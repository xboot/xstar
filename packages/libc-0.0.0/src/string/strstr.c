#include <xos/xos.h>
#include <string.h>

char * strstr(const char * s1, const char * s2)
{
	return xos_strstr(s1, s2);
}
