#include <xos/xos.h>
#include <stdio.h>

int64_t fseek(FILE * f, int64_t off, int whence)
{
	if(f)
	{
		int64_t offset = 0;
		switch(whence)
		{
		case SEEK_SET:
			offset = off;
			break;
		case SEEK_CUR:
			offset = off + xos_file_tell(f->fd);
			break;
		case SEEK_END:
			offset = xos_file_length(f->fd) - off;
			break;
		default:
			break;
		}
		return xos_file_seek(f->fd, offset);
	}
	return 0;
}
