#include <xos/xos.h>
#include <malloc.h>

void * memalign(size_t align, size_t size)
{
	return xos_mem_memalign(align, size);
}
