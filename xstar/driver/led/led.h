#ifndef __XSTAR_DRIVER_LED_H__
#define __XSTAR_DRIVER_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct led_t {
	/* The LED name */
	char * name;

	/* Set LED's brightness (0 ~ 1000) */
	void (*set)(struct led_t * led, int brightness);

	/* Get LED's brightness */
	int (*get)(struct led_t * led);

	/* Private data */
	void * priv;
};

struct led_t * search_led(const char * name);
struct device_t * register_led(struct led_t * led, struct driver_t * drv);
void unregister_led(struct led_t * led);

void led_set_brightness(struct led_t * led, int brightness);
int led_get_brightness(struct led_t * led);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_LED_H__ */
