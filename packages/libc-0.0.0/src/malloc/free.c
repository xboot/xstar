#include <xos/xos.h>
#include <malloc.h>

void free(void * ptr)
{
	return xos_mem_free(ptr);
}
