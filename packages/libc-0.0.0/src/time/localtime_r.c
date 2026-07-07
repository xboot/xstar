#include <kernel/core/setting.h>
#include <time.h>

struct tm * localtime_r(const time_t * t, struct tm * tm)
{
	if(__secs_to_tm(*t + timezone(setting_get("timezone", NULL)), tm) < 0)
		return NULL;
	return tm;
}
