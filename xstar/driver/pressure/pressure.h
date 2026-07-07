#ifndef __XSTAR_DRIVER_PRESSURE_H__
#define __XSTAR_DRIVER_PRESSURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct pressure_t {
	char * name;

	int (*get)(struct pressure_t * p, int * pressure, int * temperature);
	void * priv;
};

struct pressure_t * search_pressure(const char * name);
struct pressure_t * search_first_pressure(void);
struct device_t * register_pressure(struct pressure_t * p, struct driver_t * drv);
void unregister_pressure(struct pressure_t * p);

int pressure_get(struct pressure_t * p, int * pressure, int * temperature);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_PRESSURE_H__ */
