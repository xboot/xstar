#include <xos/xos.h>
#include <string.h>

char * strrchr(const char * s, int c)
{
	return xos_strrchr(s, c);
}
