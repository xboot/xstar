#include <xos/xos.h>
#include <time.h>

char * asctime(const struct tm * tm)
{
	static char buf[32];
	return asctime_r(tm, buf);
}
