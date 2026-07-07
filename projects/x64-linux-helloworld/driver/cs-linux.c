#include <linux/linux.h>

static uint64_t cs_linux_read(struct clocksource_t * cs)
{
	return linux_timer_count();
}

static struct device_t * cs_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clocksource_t * cs;
	struct device_t * dev;

	cs = xos_mem_malloc(sizeof(struct clocksource_t));
	if(!cs)
		return NULL;

	if(linux_timer_frequency() != 1000000000ULL)
	{
		clocksource_calc_mult_shift(&cs->mult, &cs->shift, linux_timer_frequency(), 1000000000ULL, 60);
	}
	else
	{
		cs->mult = 1;
		cs->shift = 0;
	}
	cs->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	cs->mask = CLOCKSOURCE_MASK(63);
	cs->read = cs_linux_read;
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

static void cs_linux_remove(struct device_t * dev)
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

static void cs_linux_suspend(struct device_t * dev)
{
}

static void cs_linux_resume(struct device_t * dev)
{
}

static struct driver_t cs_linux = {
	.name		= "cs-linux",
	.probe		= cs_linux_probe,
	.remove		= cs_linux_remove,
	.suspend	= cs_linux_suspend,
	.resume		= cs_linux_resume,
};

static void cs_linux_driver_init(void)
{
	register_driver(&cs_linux);
}

static void cs_linux_driver_exit(void)
{
	unregister_driver(&cs_linux);
}

driver_initcall(cs_linux_driver_init);
driver_exitcall(cs_linux_driver_exit);
