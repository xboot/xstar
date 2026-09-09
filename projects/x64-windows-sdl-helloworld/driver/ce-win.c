#include <win/win.h>

static void ce_win_interrupt(void * data)
{
	struct clockevent_t * ce = (struct clockevent_t *)data;
	ce->handler(ce, ce->data);
}

static int ce_win_next(struct clockevent_t * ce, uint64_t evt)
{
	win_timer_next(evt & 0xffffffffffffffffULL, ce_win_interrupt, ce);
	return TRUE;
}

static struct device_t * ce_win_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clockevent_t * ce;
	struct device_t * dev;
	uint64_t rate = win_timer_frequency();

	if(rate == 0)
		return NULL;

	ce = xos_mem_malloc(sizeof(struct clockevent_t));
	if(!ce)
		return NULL;

	clockevent_calc_mult_shift(ce, rate, (uint32_t)XCLAMP(0xffffffffffffffffULL / rate, (uint64_t)1, (uint64_t)600));
	ce->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	ce->min_delta_ns = clockevent_delta2ns(ce, 0x1);
	ce->max_delta_ns = clockevent_delta2ns(ce, 0xffffffffffffffffULL);
	ce->next = ce_win_next;
	ce->priv = 0;
	win_timer_init();

	if(!(dev = register_clockevent(ce, drv)))
	{
		win_timer_exit();
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
		return NULL;
	}
	return dev;
}

static void ce_win_remove(struct device_t * dev)
{
	struct clockevent_t * ce = (struct clockevent_t *)dev->priv;

	if(ce)
	{
		unregister_clockevent(ce);
		win_timer_exit();
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
	}
}

static void ce_win_suspend(struct device_t * dev)
{
}

static void ce_win_resume(struct device_t * dev)
{
}

static struct driver_t ce_win = {
	.name		= "ce-win",
	.probe		= ce_win_probe,
	.remove		= ce_win_remove,
	.suspend	= ce_win_suspend,
	.resume		= ce_win_resume,
};

static void ce_win_driver_init(void)
{
	register_driver(&ce_win);
}

static void ce_win_driver_exit(void)
{
	unregister_driver(&ce_win);
}

driver_initcall(ce_win_driver_init);
driver_exitcall(ce_win_driver_exit);
