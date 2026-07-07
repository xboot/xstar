#include <xos/xos.h>
#include <stdio.h>

int vsscanf(const char * buf, const char * fmt, va_list ap)
{
	return xos_vsscanf(buf, fmt, ap);
}
