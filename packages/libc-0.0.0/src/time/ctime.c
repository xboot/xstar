#include <xos/xos.h>
#include <time.h>

char * ctime(const time_t * t)
{
	struct tm * tm = localtime(t);
	if(!tm)
		return NULL;
	return asctime(tm);
}
