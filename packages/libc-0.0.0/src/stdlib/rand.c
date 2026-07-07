#include <xos/xos.h>
#include <stdlib.h>

int rand(void)
{
	return xos_rand();
}

void srand(unsigned int seed)
{
	xos_srand(seed);
}
