#include <linux/linux.h>

struct rtc_linux_pdata_t {
	char * path;
};

static int rtc_linux_settime(struct rtc_t * rtc, struct rtc_time_t * time)
{
	struct rtc_linux_pdata_t * pdat = (struct rtc_linux_pdata_t *)rtc->priv;
	struct linux_rtc_time_t tm;

	tm.second = time->second;
	tm.minute = time->minute;
	tm.hour = time->hour;
	tm.week = time->week;
	tm.day = time->day;
	tm.month = time->month;
	tm.year = time->year;
	return linux_rtc_settime(pdat->path, &tm);
}

static int rtc_linux_gettime(struct rtc_t * rtc, struct rtc_time_t * time)
{
	struct rtc_linux_pdata_t * pdat = (struct rtc_linux_pdata_t *)rtc->priv;
	struct linux_rtc_time_t tm;

	if(linux_rtc_gettime(pdat->path, &tm))
	{
		time->second = tm.second;
		time->minute = tm.minute;
		time->hour = tm.hour;
		time->week = tm.week;
		time->day = tm.day;
		time->month = tm.month;
		time->year = tm.year;
		return TRUE;
	}
	return FALSE;
}

static struct device_t * rtc_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct rtc_linux_pdata_t * pdat;
	struct rtc_t * rtc;
	struct device_t * dev;
	char * path = dt_read_string(n, "device", NULL);

	if(!linux_rtc_exist(path))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct rtc_linux_pdata_t));
	if(!pdat)
		return NULL;

	rtc = xos_mem_malloc(sizeof(struct rtc_t));
	if(!rtc)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->path = xos_strdup(path);

	rtc->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	rtc->settime = rtc_linux_settime;
	rtc->gettime = rtc_linux_gettime;
	rtc->priv = pdat;

	if(!(dev = register_rtc(rtc, drv)))
	{
		xos_mem_free(pdat->path);
		free_device_name(rtc->name);
		xos_mem_free(rtc->priv);
		xos_mem_free(rtc);
		return NULL;
	}
	return dev;
}

static void rtc_linux_remove(struct device_t * dev)
{
	struct rtc_t * rtc = (struct rtc_t *)dev->priv;
	struct rtc_linux_pdata_t * pdat = (struct rtc_linux_pdata_t *)rtc->priv;

	if(rtc)
	{
		unregister_rtc(rtc);
		xos_mem_free(pdat->path);
		free_device_name(rtc->name);
		xos_mem_free(rtc->priv);
		xos_mem_free(rtc);
	}
}

static void rtc_linux_suspend(struct device_t * dev)
{
}

static void rtc_linux_resume(struct device_t * dev)
{
}

static struct driver_t rtc_linux = {
	.name		= "rtc-linux",
	.probe		= rtc_linux_probe,
	.remove		= rtc_linux_remove,
	.suspend	= rtc_linux_suspend,
	.resume		= rtc_linux_resume,
};

static void rtc_linux_driver_init(void)
{
	register_driver(&rtc_linux);
}

static void rtc_linux_driver_exit(void)
{
	unregister_driver(&rtc_linux);
}

driver_initcall(rtc_linux_driver_init);
driver_exitcall(rtc_linux_driver_exit);
