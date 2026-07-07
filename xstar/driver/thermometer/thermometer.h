#ifndef __XSTAR_DRIVER_THERMOMETER_H__
#define __XSTAR_DRIVER_THERMOMETER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct thermometer_t {
	char * name;

	int (*get)(struct thermometer_t * thermometer, int * temperature);
	void * priv;
};

struct thermometer_t * search_thermometer(const char * name);
struct thermometer_t * search_first_thermometer(void);
struct device_t * register_thermometer(struct thermometer_t * thermometer, struct driver_t * drv);
void unregister_thermometer(struct thermometer_t * thermometer);

int thermometer_get(struct thermometer_t * thermometer, int * temperature);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_THERMOMETER_H__ */
