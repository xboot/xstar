#include <xos/xos.h>
#include <string.h>

void * memcpy(void * dest, const void * src, size_t len)
{
	return xos_memcpy(dest, src, len);
}
