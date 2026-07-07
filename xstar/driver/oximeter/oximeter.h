#ifndef __XSTAR_DRIVER_OXIMETER_H__
#define __XSTAR_DRIVER_OXIMETER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct oximeter_t {
	char * name;

	int (*get)(struct oximeter_t * oximeter, int * spo2, int * heartrate);
	void * priv;
};

struct oximeter_t * search_oximeter(const char * name);
struct oximeter_t * search_first_oximeter(void);
struct device_t * register_oximeter(struct oximeter_t * oximeter, struct driver_t * drv);
void unregister_oximeter(struct oximeter_t * oximeter);

int oximeter_get(struct oximeter_t * oximeter, int * spo2, int * heartrate);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_OXIMETER_H__ */
