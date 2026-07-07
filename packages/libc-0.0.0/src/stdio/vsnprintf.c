#include <xos/xos.h>
#include <stdio.h>

int vsnprintf(char * buf, size_t n, const char * fmt, va_list ap)
{
	return xos_vsnprintf(buf, n, fmt, ap);
}
