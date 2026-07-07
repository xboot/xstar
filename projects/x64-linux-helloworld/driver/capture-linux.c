#include <linux/linux.h>

struct capture_linux_pdata_t
{
	void * ctx;
};

static int capture_linux_start(struct audio_capture_t * capture, int rate, int channel)
{
	struct capture_linux_pdata_t * pdat = (struct capture_linux_pdata_t *)capture->priv;

	if(!pdat->ctx)
		pdat->ctx = linux_audio_capture_start(rate, channel);
	return pdat->ctx ? 1 : 0;
}

static int capture_linux_read(struct audio_capture_t * capture, float * samples, int nsample)
{
	struct capture_linux_pdata_t * pdat = (struct capture_linux_pdata_t *)capture->priv;

	if(pdat->ctx)
		return linux_audio_capture_read(pdat->ctx, samples, nsample);
	return 0;
}

static int capture_linux_stop(struct audio_capture_t * capture)
{
	struct capture_linux_pdata_t * pdat = (struct capture_linux_pdata_t *)capture->priv;

	if(pdat->ctx)
	{
		linux_audio_capture_stop(pdat->ctx);
		pdat->ctx = NULL;
	}
	return !pdat->ctx ? 1 : 0;
}

static int capture_linux_ioctl(struct audio_capture_t * capture, const char * cmd, void * arg)
{
	switch(shash(cmd))
	{
	case 0x96f92fd6: /* "audio-capture-set-volume" */
		if(arg)
		{
			int * p = arg;
			linux_audio_capture_set_volume(p[0]);
			return 0;
		}
		break;

	case 0x667a624a: /* "audio-capture-get-volume" */
		if(arg)
		{
			int * p = arg;
			p[0] = linux_audio_capture_get_volume();
			return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static struct device_t * capture_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct capture_linux_pdata_t * pdat;
	struct audio_capture_t * capture;
	struct device_t * dev;

	pdat = xos_mem_malloc(sizeof(struct capture_linux_pdata_t));
	if(!pdat)
		return NULL;

	capture = xos_mem_malloc(sizeof(struct audio_capture_t));
	if(!capture)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->ctx = NULL;

	capture->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	capture->start = capture_linux_start;
	capture->read = capture_linux_read;
	capture->stop = capture_linux_stop;
	capture->ioctl = capture_linux_ioctl;
	capture->priv = pdat;

	if(!(dev = register_audio_capture(capture, drv)))
	{
		if(pdat->ctx)
			linux_audio_capture_stop(pdat->ctx);
		free_device_name(capture->name);
		xos_mem_free(capture->priv);
		xos_mem_free(capture);
		return NULL;
	}
	return dev;
}

static void capture_linux_remove(struct device_t * dev)
{
	struct audio_capture_t * capture = (struct audio_capture_t *)dev->priv;
	struct capture_linux_pdata_t * pdat = (struct capture_linux_pdata_t *)capture->priv;

	if(capture)
	{
		unregister_audio_capture(capture);
		if(pdat->ctx)
			linux_audio_capture_stop(pdat->ctx);
		free_device_name(capture->name);
		xos_mem_free(capture->priv);
		xos_mem_free(capture);
	}
}

static void capture_linux_suspend(struct device_t * dev)
{
}

static void capture_linux_resume(struct device_t * dev)
{
}

static struct driver_t capture_linux = {
	.name		= "capture-linux",
	.probe		= capture_linux_probe,
	.remove		= capture_linux_remove,
	.suspend	= capture_linux_suspend,
	.resume		= capture_linux_resume,
};

static void capture_linux_driver_init(void)
{
	register_driver(&capture_linux);
}

static void capture_linux_driver_exit(void)
{
	unregister_driver(&capture_linux);
}

driver_initcall(capture_linux_driver_init);
driver_exitcall(capture_linux_driver_exit);
