#ifndef __XSTAR_DRIVER_RNG_H__
#define __XSTAR_DRIVER_RNG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct rng_t {
	char * name;

	int (*read)(struct rng_t * rng, void * buf, int len, int timeout);
	void * priv;
};

struct rng_t * search_rng(const char * name);
struct rng_t * search_first_rng(void);
struct device_t * register_rng(struct rng_t * rng, struct driver_t * drv);
void unregister_rng(struct rng_t * rng);
int rng_read(struct rng_t * rng, void * buf, int len, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_RNG_H__ */
