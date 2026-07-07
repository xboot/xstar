#include <xos/xos.h>
#include <stdio.h>

int fclose(FILE * f)
{
	int ret = -1;

	if(f)
	{
		ret = xos_file_close(f->fd);
		xos_mem_free(f);
	}
	return ret;
}
