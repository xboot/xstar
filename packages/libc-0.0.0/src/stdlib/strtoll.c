#include <xos/xos.h>
#include <stdlib.h>

long long strtoll(const char * nptr, char ** endptr, int base)
{
	return xos_strtoll(nptr, endptr, base);
}
