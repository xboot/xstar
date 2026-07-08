#include <linux/linux.h>

struct led_linux_pdata_t {
	char * path;
	int brightness;
	int max_brightness;
};

static void led_linux_set(struct led_t * led, int brightness)
{
	struct led_linux_pdata_t * pdat = (struct led_linux_pdata_t *)led->priv;
	char tmp[32];
	int v;

	if(pdat->brightness != brightness)
	{
		v = brightness * pdat->max_brightness / 1000;
		xos_sprintf(tmp, "%d", v);
		linux_sysfs_write_string(pdat->path, tmp);
		pdat->brightness = brightness;
	}
}

static int led_linux_get(struct led_t * led)
{
	struct led_linux_pdata_t * pdat = (struct led_linux_pdata_t *)led->priv;
	return pdat->brightness;
}

static struct device_t * led_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct led_linux_pdata_t * pdat;
	struct led_t * led;
	struct device_t * dev;
	char tpath[512], tmp[32];
	char * path = dt_read_string(n, "path", NULL);
	int brightness = -1;
	int max_brightness = -1;

	if(!path || !linux_sysfs_access(path, "rw"))
		return NULL;

	xos_strlcpy(tpath, path, sizeof(tpath));
	path_dirname(tpath);
	xos_strlcat(tpath, "/max_brightness", sizeof(tpath));
	if(linux_sysfs_read_string(tpath, tmp))
		max_brightness = xos_strtol(tmp, NULL, 0);
	if(max_brightness <= 0)
		return NULL;

	if(linux_sysfs_read_string(path, tmp))
	{
		brightness = xos_strtol(tmp, NULL, 0);
		if(max_brightness > 0)
			brightness = brightness * 1000 / max_brightness > 0;
	}

	pdat = xos_mem_malloc(sizeof(struct led_linux_pdata_t));
	if(!pdat)
		return NULL;

	led = xos_mem_malloc(sizeof(struct led_t));
	if(!led)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->path = xos_strdup(path);
	pdat->brightness = brightness;
	pdat->max_brightness = max_brightness;

	led->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	led->set = led_linux_set;
	led->get = led_linux_get;
	led->priv = pdat;

	if(!(dev = register_led(led, drv)))
	{
		if(pdat->path)
			xos_mem_free(pdat->path);
		free_device_name(led->name);
		xos_mem_free(led->priv);
		xos_mem_free(led);
		return NULL;
	}
	return dev;
}

static void led_linux_remove(struct device_t * dev)
{
	struct led_t * led = (struct led_t *)dev->priv;
	struct led_linux_pdata_t * pdat = (struct led_linux_pdata_t *)led->priv;

	if(led)
	{
		unregister_led(led);
		if(pdat->path)
			xos_mem_free(pdat->path);
		free_device_name(led->name);
		xos_mem_free(led->priv);
		xos_mem_free(led);
	}
}

static void led_linux_suspend(struct device_t * dev)
{
}

static void led_linux_resume(struct device_t * dev)
{
}

static struct driver_t led_linux = {
	.name		= "led-linux",
	.probe		= led_linux_probe,
	.remove		= led_linux_remove,
	.suspend	= led_linux_suspend,
	.resume		= led_linux_resume,
};

static void led_linux_driver_init(void)
{
	register_driver(&led_linux);
}

static void led_linux_driver_exit(void)
{
	unregister_driver(&led_linux);
}

driver_initcall(led_linux_driver_init);
driver_exitcall(led_linux_driver_exit);
