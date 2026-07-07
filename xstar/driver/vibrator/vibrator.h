#ifndef __XSTAR_DRIVER_VIBRATOR_H__
#define __XSTAR_DRIVER_VIBRATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <kernel/core/thworker.h>

struct vibrator_t {
	char * name;

	void (*set)(struct vibrator_t * vib, int state);
	int (*get)(struct vibrator_t * vib);
	void * priv;
};

struct vibrator_t * search_vibrator(const char * name);
struct vibrator_t * search_first_vibrator(void);
struct device_t * register_vibrator(struct vibrator_t * vib, struct driver_t * drv);
void unregister_vibrator(struct vibrator_t * vib);

void vibrator_set_state(struct vibrator_t * vib, int state);
int vibrator_get_state(struct vibrator_t * vib);

/*
 * vibrator async
 */
struct vibrator_async_ctx_t {
	struct vibrator_t * vibrator;
	struct thworker_t * worker;
};

struct vibrator_async_ctx_t * vibrator_async_ctx_alloc(const char * name);
void vibrator_async_ctx_free(struct vibrator_async_ctx_t * ctx);
void vibrator_async_ctx_clear(struct vibrator_async_ctx_t * ctx);

void vibrator_async_vibrate(struct vibrator_async_ctx_t * ctx, int state, int millisecond);
void vibrator_async_play(struct vibrator_async_ctx_t * ctx, const char * morse);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_VIBRATOR_H__ */
