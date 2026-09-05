#ifndef __XSTAR_DRIVER_DEVICE_H__
#define __XSTAR_DRIVER_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/driver.h>

enum device_type_t {
	DEVICE_TYPE_ADC				= 0,
	DEVICE_TYPE_ATNET			= 1,
	DEVICE_TYPE_ATOMIC			= 2,
	DEVICE_TYPE_AUDIOCAPTURE	= 3,
	DEVICE_TYPE_AUDIOPLAYBACK	= 4,
	DEVICE_TYPE_BATTERY			= 5,
	DEVICE_TYPE_BLOCK			= 6,
	DEVICE_TYPE_BUZZER			= 7,
	DEVICE_TYPE_CAMERA			= 8,
	DEVICE_TYPE_CLK				= 9,
	DEVICE_TYPE_CLOCKEVENT		= 10,
	DEVICE_TYPE_CLOCKSOURCE		= 11,
	DEVICE_TYPE_COMPASS			= 12,
	DEVICE_TYPE_CONSOLE			= 13,
	DEVICE_TYPE_DAC				= 14,
	DEVICE_TYPE_DMACHIP			= 15,
	DEVICE_TYPE_FRAMEBUFFER		= 16,
	DEVICE_TYPE_G2D				= 17,
	DEVICE_TYPE_GMETER			= 18,
	DEVICE_TYPE_GNSS			= 19,
	DEVICE_TYPE_GPIOCHIP		= 20,
	DEVICE_TYPE_GYROSCOPE		= 21,
	DEVICE_TYPE_HWSPINLOCK		= 22,
	DEVICE_TYPE_HYGROMETER		= 23,
	DEVICE_TYPE_I2C				= 24,
	DEVICE_TYPE_INPUT			= 25,
	DEVICE_TYPE_IRQCHIP			= 26,
	DEVICE_TYPE_LED				= 27,
	DEVICE_TYPE_LEDSTRIP		= 28,
	DEVICE_TYPE_LEDTRIGGER		= 29,
	DEVICE_TYPE_LIGHT			= 30,
	DEVICE_TYPE_LIMITER			= 31,
	DEVICE_TYPE_MOTOR			= 32,
	DEVICE_TYPE_NET				= 33,
	DEVICE_TYPE_NVMEM			= 34,
	DEVICE_TYPE_OXIMETER		= 35,
	DEVICE_TYPE_PRESSURE		= 36,
	DEVICE_TYPE_PRINTER			= 37,
	DEVICE_TYPE_PROXIMITY		= 38,
	DEVICE_TYPE_PWM				= 39,
	DEVICE_TYPE_REGULATOR		= 40,
	DEVICE_TYPE_RESETCHIP		= 41,
	DEVICE_TYPE_RNG				= 42,
	DEVICE_TYPE_RTC				= 43,
	DEVICE_TYPE_SDHCI			= 44,
	DEVICE_TYPE_SERVO			= 45,
	DEVICE_TYPE_SPI				= 46,
	DEVICE_TYPE_SPINLOCK		= 47,
	DEVICE_TYPE_STEPPER			= 48,
	DEVICE_TYPE_THERMOMETER		= 49,
	DEVICE_TYPE_UART			= 50,
	DEVICE_TYPE_VIBRATOR		= 51,
	DEVICE_TYPE_WATCHDOG		= 52,

	DEVICE_TYPE_MAX_COUNT		= 53,
};

struct driver_t;

struct device_t {
	struct kobj_t * kobj;
	struct list_head_t list;
	struct list_head_t head;
	struct hlist_node_t node;

	char * name;
	enum device_type_t type;
	struct driver_t * driver;
	void * priv;
};

extern struct list_head_t __device_list;
extern struct list_head_t __device_head[DEVICE_TYPE_MAX_COUNT];

char * alloc_device_name(const char * name, int id);
void free_device_name(char * name);
struct device_t * search_device(const char * name, enum device_type_t type);
struct device_t * search_first_device(enum device_type_t type);
int register_device(struct device_t * dev);
int unregister_device(struct device_t * dev);
void suspend_device(struct device_t * dev);
void resume_device(struct device_t * dev);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_DEVICE_H__ */
