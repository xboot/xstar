#include <linux/linux.h>

static int rtc_linux_soft_settime(struct rtc_t * rtc, struct rtc_time_t * time)
{
	return FALSE;
}

static int rtc_linux_soft_gettime(struct rtc_t * rtc, struct rtc_time_t * time)
{
	secs_to_rtc_time(linux_realtime() / 1000000000ULL, time);
	return TRUE;
}

static struct device_t * rtc_linux_soft_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct rtc_t * rtc;
	struct device_t * dev;

	rtc = xos_mem_malloc(sizeof(struct rtc_t));
	if(!rtc)
		return NULL;

	rtc->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	rtc->settime = rtc_linux_soft_settime;
	rtc->gettime = rtc_linux_soft_gettime;
	rtc->priv = NULL;

	if(!(dev = register_rtc(rtc, drv)))
	{
		free_device_name(rtc->name);
		xos_mem_free(rtc->priv);
		xos_mem_free(rtc);
		return NULL;
	}
	return dev;
}

static void rtc_linux_soft_remove(struct device_t * dev)
{
	struct rtc_t * rtc = (struct rtc_t *)dev->priv;

	if(rtc)
	{
		unregister_rtc(rtc);
		free_device_name(rtc->name);
		xos_mem_free(rtc->priv);
		xos_mem_free(rtc);
	}
}

static void rtc_linux_soft_suspend(struct device_t * dev)
{
}

static void rtc_linux_soft_resume(struct device_t * dev)
{
}

static struct driver_t rtc_linux_soft = {
	.name		= "rtc-linux-soft",
	.probe		= rtc_linux_soft_probe,
	.remove		= rtc_linux_soft_remove,
	.suspend	= rtc_linux_soft_suspend,
	.resume		= rtc_linux_soft_resume,
};

static void rtc_linux_soft_driver_init(void)
{
	register_driver(&rtc_linux_soft);
}

static void rtc_linux_soft_driver_exit(void)
{
	unregister_driver(&rtc_linux_soft);
}

driver_initcall(rtc_linux_soft_driver_init);
driver_exitcall(rtc_linux_soft_driver_exit);
