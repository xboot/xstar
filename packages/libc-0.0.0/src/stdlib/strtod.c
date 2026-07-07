#include <xos/xos.h>
#include <stdlib.h>

double strtod(const char * nptr, char ** endptr)
{
	return xos_strtod(nptr, endptr);
}
