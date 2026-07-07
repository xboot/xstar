#include <xos/xos.h>
#include <string.h>

char * strpbrk(const char * s1, const char * s2)
{
	return xos_strpbrk(s1, s2);
}
