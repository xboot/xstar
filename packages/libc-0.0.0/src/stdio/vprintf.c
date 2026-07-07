#include <xos/xos.h>
#include <stdio.h>

int vprintf(const char * fmt, va_list ap)
{
	return xos_vprintf(fmt, ap);
}
