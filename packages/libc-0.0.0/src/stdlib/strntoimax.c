#include <xos/xos.h>
#include <stdlib.h>

intmax_t strntoimax(const char * nptr, char ** endptr, int base, size_t n)
{
	return xos_strntoimax(nptr, endptr, base, n);
}
