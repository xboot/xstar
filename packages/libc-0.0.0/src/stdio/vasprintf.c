#include <xos/xos.h>
#include <stdio.h>

int vasprintf(char ** s, const char * fmt, va_list ap)
{
	return xos_vasprintf(s, fmt, ap);
}
