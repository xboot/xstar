#include <xos/xos.h>
#include <stdio.h>

int fscanf(FILE * f, const char * fmt, ...)
{
	if(f)
	{
		char * buf = xos_mem_calloc(1, 4096);
		if(buf)
		{
			va_list ap;
			xos_file_read(f->fd, buf, 4096);
			va_start(ap, fmt);
			int rv = xos_vsscanf(buf, fmt, ap);
			va_end(ap);
			xos_mem_free(buf);
			return rv;
		}
	}
	return 0;
}
