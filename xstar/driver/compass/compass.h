#ifndef __XSTAR_DRIVER_COMPASS_H__
#define __XSTAR_DRIVER_COMPASS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>


struct compass_t {
	char * name;
	int ox, oy, oz;

	int (*get)(struct compass_t * c, int * x, int * y, int * z);
	void * priv;
};

struct compass_t * search_compass(const char * name);
struct compass_t * search_first_compass(void);
struct device_t * register_compass(struct compass_t * c, struct driver_t * drv);
void unregister_compass(struct compass_t * c);

int compass_set_offset(struct compass_t * c, int ox, int oy, int oz);
int compass_get_offset(struct compass_t * c, int * ox, int * oy, int * oz);
int compass_get_magnetic(struct compass_t * c, int * x, int * y, int * z);
int compass_get_heading(struct compass_t * c, int * heading, int declination);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_COMPASS_H__ */
