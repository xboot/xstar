#include <kernel/time/wallclock.h>
#include <time.h>

int settimeofday(struct timeval * tv, void * tz)
{
	struct wallclock_timeval_t wtv;

	if(tv)
	{
		wtv.tv_sec = tv->tv_sec - wallclock_timezone(tz);
		wtv.tv_usec = tv->tv_usec;
		return wallclock_settimeofday(&wtv);
	}
	return 0;
}
