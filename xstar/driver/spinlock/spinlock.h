#ifndef __XSTAR_DRIVER_SPINLOCK_H__
#define __XSTAR_DRIVER_SPINLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct spinlock_t {
	volatile int lock;
};

struct spinlock_hdl_t {
	char * name;

	void (*init)(struct spinlock_hdl_t * hdl, struct spinlock_t * lock);
	int (*trylock)(struct spinlock_hdl_t * hdl, struct spinlock_t * lock);
	void (*lock)(struct spinlock_hdl_t * hdl, struct spinlock_t * lock);
	void (*unlock)(struct spinlock_hdl_t * hdl, struct spinlock_t * lock);
	void * priv;
};

struct spinlock_hdl_t * search_spinlock(const char * name);
struct spinlock_hdl_t * search_first_spinlock(void);
struct device_t * register_spinlock(struct spinlock_hdl_t * hdl, struct driver_t * drv);
void unregister_spinlock(struct spinlock_hdl_t * hdl);

void spinlock_init(struct spinlock_t * lock);
int spinlock_trylock(struct spinlock_t * lock);
void spinlock_lock(struct spinlock_t * lock);
void spinlock_unlock(struct spinlock_t * lock);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_SPINLOCK_H__ */
