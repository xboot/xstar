#include <linux/linux.h>

static void ce_linux_interrupt(void * data)
{
	struct clockevent_t * ce = (struct clockevent_t *)data;
	ce->handler(ce, ce->data);
}

static int ce_linux_next(struct clockevent_t * ce, uint64_t evt)
{
	linux_timer_next(evt & 0x7fffffffffffffff, ce_linux_interrupt, ce);
	return TRUE;
}

static struct device_t * ce_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clockevent_t * ce;
	struct device_t * dev;

	ce = xos_mem_malloc(sizeof(struct clockevent_t));
	if(!ce)
		return NULL;

	if(linux_timer_frequency() != 1000000000ULL)
	{
		clockevent_calc_mult_shift(ce, linux_timer_frequency(), 60);
	}
	else
	{
		ce->mult = 1;
		ce->shift = 0;
	}
	ce->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	ce->min_delta_ns = clockevent_delta2ns(ce, 0x1);
	ce->max_delta_ns = clockevent_delta2ns(ce, 0x7fffffffffffffff);
	ce->next = ce_linux_next;
	ce->priv = 0;
	linux_timer_init();

	if(!(dev = register_clockevent(ce, drv)))
	{
		linux_timer_exit();
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
		return NULL;
	}
	return dev;
}

static void ce_linux_remove(struct device_t * dev)
{
	struct clockevent_t * ce = (struct clockevent_t *)dev->priv;

	if(ce)
	{
		unregister_clockevent(ce);
		linux_timer_exit();
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
	}
}

static void ce_linux_suspend(struct device_t * dev)
{
}

static void ce_linux_resume(struct device_t * dev)
{
}

static struct driver_t ce_linux = {
	.name		= "ce-linux",
	.probe		= ce_linux_probe,
	.remove		= ce_linux_remove,
	.suspend	= ce_linux_suspend,
	.resume		= ce_linux_resume,
};

static void ce_linux_driver_init(void)
{
	register_driver(&ce_linux);
}

static void ce_linux_driver_exit(void)
{
	unregister_driver(&ce_linux);
}

driver_initcall(ce_linux_driver_init);
driver_exitcall(ce_linux_driver_exit);
