#ifndef __XSTAR_KERNEL_TIME_WALLCLOCK_H__
#define __XSTAR_KERNEL_TIME_WALLCLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct wallclock_timeval_t {
	int64_t tv_sec;
	int64_t tv_usec;
};

struct wallclock_time_t {
	uint8_t second;	/* second [0 - 59] */
	uint8_t minute;	/* minute [0 - 59] */
	uint8_t hour;	/* hour [0 - 23] */
	uint8_t week;	/* week [0 - 6] */
	uint8_t day;	/* day [1 - 31] */
	uint8_t month;	/* month [1 - 12] */
	uint32_t year;	/* year */
};

int wallclock_timezone(const char * tz);
int wallclock_gettimeofday(struct wallclock_timeval_t * tv);
int wallclock_settimeofday(struct wallclock_timeval_t * tv);
int wallclock_gettime(struct wallclock_time_t * tm, const char * tz);
int wallclock_settime(struct wallclock_time_t * tm, const char * tz);

void do_init_wallclock(void);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_TIME_WALLCLOCK_H__ */
