#include <xos/xos.h>
#include <stdio.h>

int snprintf(char * buf, size_t n, const char * fmt, ...)
{
	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = xos_vsnprintf(buf, n, fmt, ap);
	va_end(ap);

	return rv;
}
