#ifndef __XSTAR_DRIVER_ATOMIC_H__
#define __XSTAR_DRIVER_ATOMIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct atomic_t {
	volatile int counter;
};

struct atomic_hdl_t {
	char * name;

	void (*set)(struct atomic_hdl_t * hdl, struct atomic_t * a, int v);
	int (*get)(struct atomic_hdl_t * hdl, struct atomic_t * a);
	int (*add)(struct atomic_hdl_t * hdl, struct atomic_t * a, int v);
	int (*sub)(struct atomic_hdl_t * hdl, struct atomic_t * a, int v);
	int (*cas)(struct atomic_hdl_t * hdl, struct atomic_t * a, int o, int n);
	void * priv;
};

struct atomic_hdl_t * search_atomic(const char * name);
struct atomic_hdl_t * search_first_atomic(void);
struct device_t * register_atomic(struct atomic_hdl_t * hdl, struct driver_t * drv);
void unregister_atomic(struct atomic_hdl_t * hdl);

void atomic_set(struct atomic_t * a, int v);
int atomic_get(struct atomic_t * a);
int atomic_add(struct atomic_t * a, int v);
int atomic_sub(struct atomic_t * a, int v);
int atomic_cas(struct atomic_t * a, int o, int n);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_ATOMIC_H__ */
