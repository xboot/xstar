#include <xos/xos.h>
#include <stdlib.h>

intmax_t strtoimax(const char * nptr, char ** endptr, int base)
{
	return xos_strtoimax(nptr, endptr, base);
}
