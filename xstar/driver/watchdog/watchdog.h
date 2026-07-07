#ifndef __XSTAR_DRIVER_WATCHDOG_H__
#define __XSTAR_DRIVER_WATCHDOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct watchdog_t {
	/* The watchdog name */
	char * name;

	/* Set watchdog's timeout in seconds, zero means stop */
	void (*set)(struct watchdog_t * wdg, int timeout);

	/* Get watchdog's timeout in seconds, remaining time */
	int (*get)(struct watchdog_t * wdg);

	/* Private data */
	void * priv;
};

struct watchdog_t * search_watchdog(const char * name);
struct watchdog_t * search_first_watchdog(void);
struct device_t * register_watchdog(struct watchdog_t * wdg, struct driver_t * drv);
void unregister_watchdog(struct watchdog_t * wdg);

void watchdog_set_timeout(struct watchdog_t * wdg, int timeout);
int watchdog_get_timeout(struct watchdog_t * wdg);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_WATCHDOG_H__ */
