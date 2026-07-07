#ifndef __XSTAR_DRIVER_SERVO_H__
#define __XSTAR_DRIVER_SERVO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct servo_t {
	char * name;

	void (*enable)(struct servo_t * m);
	void (*disable)(struct servo_t * m);
	int (*range)(struct servo_t * m);
	int (*get)(struct servo_t * m);
	void (*set)(struct servo_t * m, int angle);
	void * priv;
};

struct servo_t * search_servo(const char * name);
struct device_t * register_servo(struct servo_t * m, struct driver_t * drv);
void unregister_servo(struct servo_t * m);

void servo_enable(struct servo_t * m);
void servo_disable(struct servo_t * m);
int servo_get_range(struct servo_t * m);
int servo_get_angle(struct servo_t * m);
void servo_set_angle(struct servo_t * m, int angle);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_SERVO_H__ */
