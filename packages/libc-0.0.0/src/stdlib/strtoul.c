#include <xos/xos.h>
#include <stdlib.h>

unsigned long strtoul(const char * nptr, char ** endptr, int base)
{
	return xos_strtoul(nptr, endptr, base);
}
