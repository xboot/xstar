#ifndef __XSTAR_LIBX_CRON_H__
#define __XSTAR_LIBX_CRON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

/*
 * Standard 5-field cron expression
 *   *    *    *    *    *
 *   │    │    │    │    │
 *   │    │    │    │    └── day of week (0 - 6, 0 = Sunday)
 *   │    │    │    └─────── month (1 - 12)
 *   │    │    └──────────── day of month (1 - 31)
 *   │    └───────────────── hour (0 - 23)
 *   └────────────────────── minute (0 - 59)
 */
struct cron_t {
	uint64_t minute;
	uint32_t hour;
	uint32_t day;
	uint16_t month;
	uint8_t week;
	uint8_t restricted;
};

int cron_parse(struct cron_t * cron, const char * expr);
int cron_match(struct cron_t * cron, int minute, int hour, int day, int month, int week);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_CRON_H__ */
