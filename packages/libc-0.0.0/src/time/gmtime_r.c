#include <xos/xos.h>
#include <time.h>

struct tm * gmtime_r(const time_t * t, struct tm * tm)
{
	if(__secs_to_tm(*t, tm) < 0)
		return NULL;
	return tm;
}
