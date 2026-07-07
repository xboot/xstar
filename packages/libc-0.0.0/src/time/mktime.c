#include <kernel/core/setting.h>
#include <time.h>

time_t mktime(struct tm * tm)
{
	return __tm_to_secs(tm) - timezone(setting_get("timezone", NULL));
}
