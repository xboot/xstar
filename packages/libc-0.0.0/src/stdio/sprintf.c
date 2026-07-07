#include <xos/xos.h>
#include <stdio.h>

int sprintf(char * buf, const char * fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = xos_vsnprintf(buf, ~(size_t)0, fmt, ap);
	va_end(ap);

	return rv;
}
