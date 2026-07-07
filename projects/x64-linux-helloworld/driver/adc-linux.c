#include <linux/linux.h>

struct adc_linux_pdata_t {
	char * path;
};

static int32_t adc_linux_read(struct adc_t * adc, int channel)
{
	struct adc_linux_pdata_t * pdat = (struct adc_linux_pdata_t *)adc->priv;
	char tmp[32];

	if(linux_sysfs_read_string(pdat->path, tmp))
		return xos_strtol(tmp, NULL, 0);
	return 0;
}

static struct device_t * adc_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct adc_linux_pdata_t * pdat;
	struct adc_t * adc;
	struct device_t * dev;
	char * path = dt_read_string(n, "path", NULL);

	if(!path || !linux_sysfs_access(path, "r"))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct adc_linux_pdata_t));
	if(!pdat)
		return NULL;

	adc = xos_mem_malloc(sizeof(struct adc_t));
	if(!adc)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->path = xos_strdup(path);

	adc->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	adc->vreference = dt_read_int(n, "reference-voltage", 1800000);
	adc->resolution = dt_read_int(n, "resolution", 12);
	adc->nchannel = 1;
	adc->read = adc_linux_read;
	adc->priv = pdat;

	if(!(dev = register_adc(adc, drv)))
	{
		if(pdat->path)
			xos_mem_free(pdat->path);
		free_device_name(adc->name);
		xos_mem_free(adc->priv);
		xos_mem_free(adc);
		return NULL;
	}
	return dev;
}

static void adc_linux_remove(struct device_t * dev)
{
	struct adc_t * adc = (struct adc_t *)dev->priv;
	struct adc_linux_pdata_t * pdat = (struct adc_linux_pdata_t *)adc->priv;

	if(adc)
	{
		unregister_adc(adc);
		if(pdat->path)
			xos_mem_free(pdat->path);
		free_device_name(adc->name);
		xos_mem_free(adc->priv);
		xos_mem_free(adc);
	}
}

static void adc_linux_suspend(struct device_t * dev)
{
}

static void adc_linux_resume(struct device_t * dev)
{
}

static struct driver_t adc_linux = {
	.name		= "adc-linux",
	.probe		= adc_linux_probe,
	.remove		= adc_linux_remove,
	.suspend	= adc_linux_suspend,
	.resume		= adc_linux_resume,
};

static void adc_linux_driver_init(void)
{
	register_driver(&adc_linux);
}

static void adc_linux_driver_exit(void)
{
	unregister_driver(&adc_linux);
}

driver_initcall(adc_linux_driver_init);
driver_exitcall(adc_linux_driver_exit);
