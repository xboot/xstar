#include <xos/xos.h>
#include <malloc.h>

void * malloc(size_t size)
{
	return xos_mem_malloc(size);
}
