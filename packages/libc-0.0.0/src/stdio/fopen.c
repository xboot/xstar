#include <xos/xos.h>
#include <stdio.h>

FILE * fopen(const char * path, const char * mode)
{
	FILE * f;
	int fd;

	fd = xos_file_open(path, mode);
	if(fd >= 0)
	{
		f = xos_mem_malloc(sizeof(FILE));
		if(f)
		{
			f->fd = fd;
			return f;
		}
	}
	return NULL;
}
