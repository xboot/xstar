#include <linux/linux.h>

ssize_t linux_stdio_read(void * buf, size_t count)
{
	return linux_file_read_nonblock(fileno(stdin), buf, count);
}

ssize_t linux_stdio_write(void * buf, size_t count)
{
	return linux_file_write(fileno(stdout), buf, count);
}
