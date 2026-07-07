#include <xos/xos.h>
#include <time.h>

time_t time(time_t * t)
{
	struct timeval tv;

	if(gettimeofday(&tv, NULL) != 0)
		return -1;
	if(t)
		*t = tv.tv_sec;
	return tv.tv_sec;
}
