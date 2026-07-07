#include <xos/xos.h>
#include <time.h>

struct tm * localtime(const time_t * t)
{
	static struct tm tm;
	return localtime_r(t, &tm);
}
