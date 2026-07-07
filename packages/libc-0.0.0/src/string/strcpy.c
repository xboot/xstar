#include <xos/xos.h>
#include <string.h>

char * strcpy(char * dest, const char * src)
{
	return xos_strcpy(dest, src);
}
