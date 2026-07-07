#include <xos/xos.h>
#include <stdio.h>

int printf(const char * fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = xos_vprintf(fmt, ap);
	va_end(ap);

	return len;
}
