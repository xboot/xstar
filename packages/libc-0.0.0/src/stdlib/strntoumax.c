#include <xos/xos.h>
#include <stdlib.h>

uintmax_t strntoumax(const char * nptr, char ** endptr, int base, size_t n)
{
	return xos_strntoumax(nptr, endptr, base, n);
}
