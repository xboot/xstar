#ifndef __XSTAR_DRIVER_LIMITER_H__
#define __XSTAR_DRIVER_LIMITER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct limiter_t {
	char * name;

	int (*get)(struct limiter_t * limiter);
	void * priv;
};

struct limiter_t * search_limiter(const char * name);
struct device_t * register_limiter(struct limiter_t * limiter, struct driver_t * drv);
void unregister_limiter(struct limiter_t * limiter);

int limiter_get_status(struct limiter_t * limiter);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_LIMITER_H__ */
