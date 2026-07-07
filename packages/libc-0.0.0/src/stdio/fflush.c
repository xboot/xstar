#include <xos/xos.h>
#include <stdio.h>

int fflush(FILE * f)
{
	if(f)
		xos_file_sync(f->fd);
	return 0;
}
