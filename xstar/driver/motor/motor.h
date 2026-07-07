#ifndef __XSTAR_DRIVER_MOTOR_H__
#define __XSTAR_DRIVER_MOTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <kernel/core/thworker.h>

struct motor_t {
	char * name;

	void (*enable)(struct motor_t * m);
	void (*disable)(struct motor_t * m);
	void (*set)(struct motor_t * m, int speed);
	void * priv;
};

struct motor_t * search_motor(const char * name);
struct motor_t * search_first_motor(void);
struct device_t * register_motor(struct motor_t * m, struct driver_t * drv);
void unregister_motor(struct motor_t * m);

void motor_enable(struct motor_t * m);
void motor_disable(struct motor_t * m);
void motor_set_speed(struct motor_t * m, int speed);

/*
 * motor async
 */
struct motor_async_ctx_t {
	struct motor_t * motor;
	struct thworker_t * worker;
};

struct motor_async_ctx_t * motor_async_ctx_alloc(const char * name);
void motor_async_ctx_free(struct motor_async_ctx_t * ctx);
void motor_async_ctx_clear(struct motor_async_ctx_t * ctx);

void motor_async_enable(struct motor_async_ctx_t * ctx);
void motor_async_disable(struct motor_async_ctx_t * ctx);
void motor_async_play(struct motor_async_ctx_t * ctx, int speed, int millisecond);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_MOTOR_H__ */
