#include <xos/xos.h>
#include <time.h>

char * ctime_r(const time_t * t, char * buf)
{
	struct tm tm, * ptm = localtime_r(t, &tm);
	return ptm ? asctime_r(ptm, buf) : NULL;
}
