#include <xos/xos.h>
#include <stdio.h>

int fprintf(FILE * f, const char * fmt, ...)
{
	if(f)
	{
		va_list ap;
		char * p = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&p, fmt, ap);
		va_end(ap);
		if(p && (len > 0))
		{
			len = xos_file_write(f->fd, p, len);
			xos_mem_free(p);
			return len;
		}
	}
	return 0;
}
