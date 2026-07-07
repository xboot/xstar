#include <xos/xos.h>
#include <stdio.h>

int64_t ftell(FILE * f)
{
	if(f)
		return xos_file_tell(f->fd);
	return 0;
}
