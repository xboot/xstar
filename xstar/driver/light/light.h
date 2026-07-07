#ifndef __XSTAR_DRIVER_LIGHT_H__
#define __XSTAR_DRIVER_LIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct light_t {
	char * name;

	int (*get)(struct light_t * light);
	void * priv;
};

struct light_t * search_light(const char * name);
struct light_t * search_first_light(void);
struct device_t * register_light(struct light_t * light, struct driver_t * drv);
void unregister_light(struct light_t * light);

int light_get_illuminance(struct light_t * light);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_LIGHT_H__ */
