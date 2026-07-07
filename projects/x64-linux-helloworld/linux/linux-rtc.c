#include <linux/linux.h>

int linux_rtc_exist(const char * dev)
{
    struct stat st;

    if((stat(dev, &st) == 0) && S_ISCHR(st.st_mode) && (access(dev, R_OK | W_OK) == 0))
    	return 1;
    return 0;
}

int linux_rtc_settime(const char * dev, struct linux_rtc_time_t * time)
{
	struct rtc_time tm;
	int fd;

	if(!time)
		return 0;
	fd = open(dev, O_RDONLY);
	if(fd < 2)
		return 0;
	tm.tm_sec = time->second;
	tm.tm_min = time->minute;
	tm.tm_hour = time->hour;
	tm.tm_wday = time->week;
	tm.tm_mday = time->day;
	tm.tm_mon = time->month - 1;
	tm.tm_year = time->year - 1900;
	if(ioctl(fd, RTC_SET_TIME, &tm) != 0)
	{
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
}

int linux_rtc_gettime(const char * dev, struct linux_rtc_time_t * time)
{
	struct rtc_time tm;
	int fd;

	fd = open(dev, O_RDONLY);
	if(fd < 2)
		return 0;
	if(ioctl(fd, RTC_RD_TIME, &tm) != 0)
	{
		close(fd);
		return 0;
	}
	if(time)
	{
		time->second = tm.tm_sec;
		time->minute = tm.tm_min;
		time->hour = tm.tm_hour;
		time->week = tm.tm_wday;
		time->day = tm.tm_mday;
		time->month = tm.tm_mon + 1;
		time->year = tm.tm_year + 1900;
	}
	close(fd);
	return 1;
}
