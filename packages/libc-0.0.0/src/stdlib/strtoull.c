#include <xos/xos.h>
#include <stdlib.h>

unsigned long long strtoull(const char * nptr, char ** endptr, int base)
{
	return xos_strtoull(nptr, endptr, base);
}
