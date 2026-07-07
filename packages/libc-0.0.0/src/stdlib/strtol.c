#include <xos/xos.h>
#include <stdlib.h>

long strtol(const char * nptr, char ** endptr, int base)
{
	return xos_strtol(nptr, endptr, base);
}
