#include <xos/xos.h>
#include <stdlib.h>

void qsort(void * aa, size_t n, size_t es, int (*cmp)(const void *, const void *))
{
	xos_qsort(aa, n, es, cmp);
}
