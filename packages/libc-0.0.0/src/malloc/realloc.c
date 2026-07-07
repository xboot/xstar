#include <xos/xos.h>
#include <malloc.h>

void * realloc(void * ptr, size_t size)
{
	return xos_mem_realloc(ptr, size);
}
