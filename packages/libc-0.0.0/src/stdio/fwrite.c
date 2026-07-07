#include <xos/xos.h>
#include <stdio.h>

size_t fwrite(const void * buf, size_t size, size_t count, FILE * f)
{
	if(f)
	{
		size_t l = size * count;
		size_t r = xos_file_write(f->fd, (const unsigned char *)buf, l);
		return r / size;
	}
	return 0;
}
