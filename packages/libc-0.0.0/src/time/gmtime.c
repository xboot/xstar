#include <xos/xos.h>
#include <time.h>

struct tm * gmtime(const time_t * t)
{
	static struct tm tm;
	return gmtime_r(t, &tm);
}
