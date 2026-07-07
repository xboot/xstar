#ifndef __XSTAR_DRIVER_BUZZER_H__
#define __XSTAR_DRIVER_BUZZER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <kernel/core/thworker.h>

struct buzzer_t {
	char * name;

	void (*set)(struct buzzer_t * buzzer, int frequency);
	int (*get)(struct buzzer_t * buzzer);
	void * priv;
};

struct buzzer_t * search_buzzer(const char * name);
struct buzzer_t * search_first_buzzer(void);
struct device_t * register_buzzer(struct buzzer_t * buzzer, struct driver_t * drv);
void unregister_buzzer(struct buzzer_t * buzzer);

void buzzer_set_frequency(struct buzzer_t * buzzer, int frequency);
int buzzer_get_frequency(struct buzzer_t * buzzer);

/*
 * buzzer async
 */
struct buzzer_async_ctx_t {
	struct buzzer_t * buzzer;
	struct thworker_t * worker;
};

struct buzzer_async_ctx_t * buzzer_async_ctx_alloc(const char * name);
void buzzer_async_ctx_free(struct buzzer_async_ctx_t * ctx);
void buzzer_async_ctx_clear(struct buzzer_async_ctx_t * ctx);

void buzzer_async_beep(struct buzzer_async_ctx_t * ctx, int frequency, int millisecond);
void buzzer_async_play(struct buzzer_async_ctx_t * ctx, const char * rtttl);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_BUZZER_H__ */
