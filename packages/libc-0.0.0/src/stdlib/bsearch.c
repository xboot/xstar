#include <xos/xos.h>
#include <stdlib.h>

void * bsearch(const void * key, const void * base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *))
{
	return xos_bsearch(key, base, nmemb, size, cmp);
}
