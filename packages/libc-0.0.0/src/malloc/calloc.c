#include <xos/xos.h>
#include <malloc.h>

void * calloc(size_t nmemb, size_t size)
{
	return xos_mem_calloc(nmemb, size);
}
