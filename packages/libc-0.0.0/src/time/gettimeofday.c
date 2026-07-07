#include <kernel/time/wallclock.h>
#include <time.h>

int gettimeofday(struct timeval * tv, void * tz)
{
	struct wallclock_timeval_t wtv;

	if(tv && wallclock_gettimeofday(&wtv))
	{
		tv->tv_sec = wtv.tv_sec + wallclock_timezone(tz);
		tv->tv_usec = wtv.tv_usec;
		return 1;
	}
	return 0;
}
