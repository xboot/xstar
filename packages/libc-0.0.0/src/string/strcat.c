#include <xos/xos.h>
#include <string.h>

char * strcat(char * dest, const char * src)
{
	return xos_strcat(dest, src);
}
