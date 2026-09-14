#include <freertos/freertos.h>

static uint64_t cs_freertos_read(struct clocksource_t * cs)
{
	return freertos_timer_count();
}

static struct device_t * cs_freertos_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clocksource_t * cs;
	struct device_t * dev;
	uint64_t rate = freertos_timer_frequency();

	if(rate == 0)
		return NULL;

	cs = xos_mem_malloc(sizeof(struct clocksource_t));
	if(!cs)
		return NULL;

	cs->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	cs->mask = CLOCKSOURCE_MASK(64);
	clocksource_calc_mult_shift(&cs->mult, &cs->shift, rate, 1000000000ULL, (uint32_t)XCLAMP((uint64_t)(cs->mask / rate), (uint64_t)1, (uint64_t)600));
	cs->read = cs_freertos_read;
	cs->priv = 0;

	if(!(dev = register_clocksource(cs, drv)))
	{
		free_device_name(cs->name);
		xos_mem_free(cs->priv);
		xos_mem_free(cs);
		return NULL;
	}
	return dev;
}

static void cs_freertos_remove(struct device_t * dev)
{
	struct clocksource_t * cs = (struct clocksource_t *)dev->priv;

	if(cs)
	{
		unregister_clocksource(cs);
		free_device_name(cs->name);
		xos_mem_free(cs->priv);
		xos_mem_free(cs);
	}
}

static void cs_freertos_suspend(struct device_t * dev)
{
}

static void cs_freertos_resume(struct device_t * dev)
{
}

static struct driver_t cs_freertos = {
	.name		= "cs-freertos",
	.probe		= cs_freertos_probe,
	.remove		= cs_freertos_remove,
	.suspend	= cs_freertos_suspend,
	.resume		= cs_freertos_resume,
};

static void cs_freertos_driver_init(void)
{
	register_driver(&cs_freertos);
}

static void cs_freertos_driver_exit(void)
{
	unregister_driver(&cs_freertos);
}

driver_initcall(cs_freertos_driver_init);
driver_exitcall(cs_freertos_driver_exit);
