#include <xos/xos.h>
#include <stdlib.h>

uintmax_t strtoumax(const char * nptr, char ** endptr, int base)
{
	return xos_strtoumax(nptr, endptr, base);
}
