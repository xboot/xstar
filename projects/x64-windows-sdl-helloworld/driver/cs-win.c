#include <win/win.h>

static uint64_t cs_win_read(struct clocksource_t * cs)
{
	return win_timer_count();
}

static struct device_t * cs_win_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clocksource_t * cs;
	struct device_t * dev;

	cs = xos_mem_malloc(sizeof(struct clocksource_t));
	if(!cs)
		return NULL;

	if(win_timer_frequency() != 1000000000ULL)
	{
		clocksource_calc_mult_shift(&cs->mult, &cs->shift, win_timer_frequency(), 1000000000ULL, 60);
	}
	else
	{
		cs->mult = 1;
		cs->shift = 0;
	}
	cs->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	cs->mask = CLOCKSOURCE_MASK(63);
	cs->read = cs_win_read;
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

static void cs_win_remove(struct device_t * dev)
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

static void cs_win_suspend(struct device_t * dev)
{
}

static void cs_win_resume(struct device_t * dev)
{
}

static struct driver_t cs_win = {
	.name		= "cs-win",
	.probe		= cs_win_probe,
	.remove		= cs_win_remove,
	.suspend	= cs_win_suspend,
	.resume		= cs_win_resume,
};

static void cs_win_driver_init(void)
{
	register_driver(&cs_win);
}

static void cs_win_driver_exit(void)
{
	unregister_driver(&cs_win);
}

driver_initcall(cs_win_driver_init);
driver_exitcall(cs_win_driver_exit);
