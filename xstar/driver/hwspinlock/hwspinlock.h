#ifndef __XSTAR_DRIVER_HWSPINLOCK_H__
#define __XSTAR_DRIVER_HWSPINLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct hwspinlock_t {
	char * name;
	int base;
	int nlock;

	int (*trylock)(struct hwspinlock_t * hsl, int lock);
	void (*lock)(struct hwspinlock_t * hsl, int lock);
	void (*unlock)(struct hwspinlock_t * hsl, int lock);
	void * priv;
};

struct hwspinlock_t * search_hwspinlock(int lock);
struct device_t * register_hwspinlock(struct hwspinlock_t * hsl, struct driver_t * drv);
void unregister_hwspinlock(struct hwspinlock_t * hsl);

int hwspinlock_is_valid(int lock);
int hwspinlock_trylock(int lock);
void hwspinlock_lock(int lock);
void hwspinlock_unlock(int lock);

struct hwspinlock_desc_t {
	struct hwspinlock_t * hsl;
	int offset;
};

struct hwspinlock_desc_t * hwspinlock_desc_alloc(int lock);
void hwspinlock_desc_free(struct hwspinlock_desc_t * desc);

static inline int hwspinlock_desc_trylock(struct hwspinlock_desc_t * desc)
{
	return desc->hsl->trylock(desc->hsl, desc->offset);
}

static inline void hwspinlock_desc_lock(struct hwspinlock_desc_t * desc)
{
	desc->hsl->lock(desc->hsl, desc->offset);
}

static inline void hwspinlock_desc_unlock(struct hwspinlock_desc_t * desc)
{
	desc->hsl->unlock(desc->hsl, desc->offset);
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_HWSPINLOCK_H__ */
