#ifndef __XSTAR_DRIVER_GPIO_H__
#define __XSTAR_DRIVER_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

enum gpio_pull_t {
	GPIO_PULL_UP			= 0,
	GPIO_PULL_DOWN			= 1,
	GPIO_PULL_NONE			= 2,
};

enum gpio_drv_t {
	GPIO_DRV_WEAK			= 0,
	GPIO_DRV_WEAKER			= 1,
	GPIO_DRV_STRONGER		= 2,
	GPIO_DRV_STRONG			= 3,
};

enum gpio_rate_t {
	GPIO_RATE_SLOW			= 0,
	GPIO_RATE_FAST			= 1,
};

enum gpio_direction_t {
	GPIO_DIRECTION_INPUT	= 0,
	GPIO_DIRECTION_OUTPUT	= 1,
};

struct gpiochip_t {
	char * name;
	int base;
	int ngpio;

	void (*set_cfg)(struct gpiochip_t * chip, int offset, int cfg);
	int  (*get_cfg)(struct gpiochip_t * chip, int offset);
	void (*set_pull)(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull);
	enum gpio_pull_t (*get_pull)(struct gpiochip_t * chip, int offset);
	void (*set_drv)(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv);
	enum gpio_drv_t (*get_drv)(struct gpiochip_t * chip, int offset);
	void (*set_rate)(struct gpiochip_t * chip, int offset, enum gpio_rate_t rate);
	enum gpio_rate_t (*get_rate)(struct gpiochip_t * chip, int offset);
	void (*set_dir)(struct gpiochip_t * chip, int offset, enum gpio_direction_t dir);
	enum gpio_direction_t (*get_dir)(struct gpiochip_t * chip, int offset);
	void (*set_value)(struct gpiochip_t * chip, int offset, int value);
	int  (*get_value)(struct gpiochip_t * chip, int offset);
	int  (*to_irq)(struct gpiochip_t * chip, int offset);
	void * priv;
};

struct gpiochip_t * search_gpiochip(int gpio);
struct device_t * register_gpiochip(struct gpiochip_t * chip, struct driver_t * drv);
void unregister_gpiochip(struct gpiochip_t * chip);

int gpio_is_valid(int gpio);
void gpio_set_cfg(int gpio, int cfg);
int gpio_get_cfg(int gpio);
void gpio_set_pull(int gpio, enum gpio_pull_t pull);
enum gpio_pull_t gpio_get_pull(int gpio);
void gpio_set_drv(int gpio, enum gpio_drv_t drv);
enum gpio_drv_t gpio_get_drv(int gpio);
void gpio_set_rate(int gpio, enum gpio_rate_t rate);
enum gpio_rate_t gpio_get_rate(int gpio);
void gpio_set_direction(int gpio, enum gpio_direction_t dir);
enum gpio_direction_t gpio_get_direction(int gpio);
void gpio_set_value(int gpio, int value);
int gpio_get_value(int gpio);
void gpio_direction_output(int gpio, int value);
int gpio_direction_input(int gpio);
int gpio_to_irq(int gpio);

struct gpiodesc_t {
	struct gpiochip_t * chip;
	int offset;
};

struct gpiodesc_t * gpiod_alloc(int gpio);
void gpiod_free(struct gpiodesc_t * desc);

static inline void gpiod_set_cfg(struct gpiodesc_t * desc, int cfg)
{
	desc->chip->set_cfg(desc->chip, desc->offset, cfg);
}

static inline int gpiod_get_cfg(struct gpiodesc_t * desc)
{
	return desc->chip->get_cfg(desc->chip, desc->offset);
}

static inline void gpiod_set_pull(struct gpiodesc_t * desc, enum gpio_pull_t pull)
{
	desc->chip->set_pull(desc->chip, desc->offset, pull);
}

static inline enum gpio_pull_t gpiod_get_pull(struct gpiodesc_t * desc)
{
	return desc->chip->get_pull(desc->chip, desc->offset);
}

static inline void gpiod_set_drv(struct gpiodesc_t * desc, enum gpio_drv_t drv)
{
	desc->chip->set_drv(desc->chip, desc->offset, drv);
}

static inline enum gpio_drv_t gpiod_get_drv(struct gpiodesc_t * desc)
{
	return desc->chip->get_drv(desc->chip, desc->offset);
}

static inline void gpiod_set_rate(struct gpiodesc_t * desc, enum gpio_rate_t rate)
{
	desc->chip->set_rate(desc->chip, desc->offset, rate);
}

static inline enum gpio_rate_t gpiod_get_rate(struct gpiodesc_t * desc)
{
	return desc->chip->get_rate(desc->chip, desc->offset);
}

static inline void gpiod_set_direction(struct gpiodesc_t * desc, enum gpio_direction_t dir)
{
	desc->chip->set_dir(desc->chip, desc->offset, dir);
}

static inline enum gpio_direction_t gpiod_get_direction(struct gpiodesc_t * desc)
{
	return desc->chip->get_dir(desc->chip, desc->offset);
}

static inline void gpiod_set_value(struct gpiodesc_t * desc, int value)
{
	desc->chip->set_value(desc->chip, desc->offset, value);
}

static inline int gpiod_get_value(struct gpiodesc_t * desc)
{
	return desc->chip->get_value(desc->chip, desc->offset);
}

static inline void gpiod_direction_output(struct gpiodesc_t * desc, int value)
{
	desc->chip->set_dir(desc->chip, desc->offset, GPIO_DIRECTION_OUTPUT);
	desc->chip->set_value(desc->chip, desc->offset, value);
}

static inline int gpiod_direction_input(struct gpiodesc_t * desc)
{
	desc->chip->set_dir(desc->chip, desc->offset, GPIO_DIRECTION_INPUT);
	return desc->chip->get_value(desc->chip, desc->offset);
}

static inline int gpiod_to_gpio(struct gpiodesc_t * desc)
{
	return desc->chip->base + desc->offset;
}

static inline int gpiod_to_irq(struct gpiodesc_t * desc)
{
	return desc->chip->to_irq(desc->chip, desc->offset);
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_GPIO_H__ */
