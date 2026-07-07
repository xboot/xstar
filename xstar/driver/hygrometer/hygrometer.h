#ifndef __XSTAR_DRIVER_HYGROMETER_H__
#define __XSTAR_DRIVER_HYGROMETER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct hygrometer_t {
	char * name;

	int (*get)(struct hygrometer_t * hygrometer, int * humidity, int * temperature);
	void * priv;
};

struct hygrometer_t * search_hygrometer(const char * name);
struct hygrometer_t * search_first_hygrometer(void);
struct device_t * register_hygrometer(struct hygrometer_t * hygrometer, struct driver_t * drv);
void unregister_hygrometer(struct hygrometer_t * hygrometer);

int hygrometer_get(struct hygrometer_t * hygrometer, int * humidity, int * temperature);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_HYGROMETER_H__ */
