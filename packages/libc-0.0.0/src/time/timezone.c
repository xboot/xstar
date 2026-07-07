#include <kernel/time/wallclock.h>
#include <time.h>

int timezone(const char * tz)
{
	return wallclock_timezone(tz);
}
