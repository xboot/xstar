#include <linux/linux.h>

struct thermometer_linux_pdata_t {
	char * path;
};

static int thermometer_linux_read(struct thermometer_t * thermometer, int * temperature)
{
	struct thermometer_linux_pdata_t * pdat = (struct thermometer_linux_pdata_t *)thermometer->priv;
	char tmp[32];

	if(linux_sysfs_read_string(pdat->path, tmp))
	{
		if(temperature)
			*temperature = xos_strtol(tmp, NULL, 0);
		return 1;
	}
	return 0;
}

static struct device_t * thermometer_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct thermometer_linux_pdata_t * pdat;
	struct thermometer_t * t;
	struct device_t * dev;
	char * path = dt_read_string(n, "path", NULL);

	if(!path || !linux_sysfs_access(path, "r"))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct thermometer_linux_pdata_t));
	if(!pdat)
		return NULL;

	t = xos_mem_malloc(sizeof(struct thermometer_t));
	if(!t)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->path = xos_strdup(path);

	t->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	t->get = thermometer_linux_read;
	t->priv = pdat;

	if(!(dev = register_thermometer(t, drv)))
	{
		if(pdat->path)
			xos_mem_free(pdat->path);
		free_device_name(t->name);
		xos_mem_free(t->priv);
		xos_mem_free(t);
		return NULL;
	}
	return dev;
}

static void thermometer_linux_remove(struct device_t * dev)
{
	struct thermometer_t * thermometer = (struct thermometer_t *)dev->priv;
	struct thermometer_linux_pdata_t * pdat = (struct thermometer_linux_pdata_t *)thermometer->priv;

	if(thermometer)
	{
		unregister_thermometer(thermometer);
		if(pdat->path)
			xos_mem_free(pdat->path);
		free_device_name(thermometer->name);
		xos_mem_free(thermometer->priv);
		xos_mem_free(thermometer);
	}
}

static void thermometer_linux_suspend(struct device_t * dev)
{
}

static void thermometer_linux_resume(struct device_t * dev)
{
}

static struct driver_t thermometer_linux = {
	.name		= "thermometer-linux",
	.probe		= thermometer_linux_probe,
	.remove		= thermometer_linux_remove,
	.suspend	= thermometer_linux_suspend,
	.resume		= thermometer_linux_resume,
};

static void thermometer_linux_driver_init(void)
{
	register_driver(&thermometer_linux);
}

static void thermometer_linux_driver_exit(void)
{
	unregister_driver(&thermometer_linux);
}

driver_initcall(thermometer_linux_driver_init);
driver_exitcall(thermometer_linux_driver_exit);
