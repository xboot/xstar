#include <linux/linux.h>

struct playback_linux_pdata_t {
	void * ctx;
};

static int playback_linux_start(struct audio_playback_t * playback, int rate, int channel)
{
	struct playback_linux_pdata_t * pdat = (struct playback_linux_pdata_t *)playback->priv;

	if(!pdat->ctx)
		pdat->ctx = linux_audio_playback_start(rate, channel);
	return pdat->ctx ? 1 : 0;
}

static int playback_linux_write(struct audio_playback_t * playback, float * samples, int nsample)
{
	struct playback_linux_pdata_t * pdat = (struct playback_linux_pdata_t *)playback->priv;

	if(pdat->ctx)
		return linux_audio_playback_write(pdat->ctx, samples, nsample);
	return 0;
}

static int playback_linux_stop(struct audio_playback_t * playback)
{
	struct playback_linux_pdata_t * pdat = (struct playback_linux_pdata_t *)playback->priv;

	if(pdat->ctx)
	{
		linux_audio_playback_stop(pdat->ctx);
		pdat->ctx = NULL;
	}
	return !pdat->ctx ? 1 : 0;
}

static int playback_linux_ioctl(struct audio_playback_t * playback, const char * cmd, void * arg)
{
	switch(shash(cmd))
	{
	case 0x0e6cf669: /* "audio-playback-set-volume" */
		if(arg)
		{
			int * p = arg;
			linux_audio_playback_set_volume(p[0]);
			return 0;
		}
		break;

	case 0xddee28dd: /* "audio-playback-get-volume" */
		if(arg)
		{
			int * p = arg;
			p[0] = linux_audio_playback_get_volume();
			return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static struct device_t * playback_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct playback_linux_pdata_t * pdat;
	struct audio_playback_t * playback;
	struct device_t * dev;

	pdat = xos_mem_malloc(sizeof(struct playback_linux_pdata_t));
	if(!pdat)
		return NULL;

	playback = xos_mem_malloc(sizeof(struct audio_playback_t));
	if(!playback)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->ctx = NULL;

	playback->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	playback->start = playback_linux_start;
	playback->write = playback_linux_write;
	playback->stop = playback_linux_stop;
	playback->ioctl = playback_linux_ioctl;
	playback->priv = pdat;

	if(!(dev = register_audio_playback(playback, drv)))
	{
		if(pdat->ctx)
			linux_audio_playback_stop(pdat->ctx);
		free_device_name(playback->name);
		xos_mem_free(playback->priv);
		xos_mem_free(playback);
		return NULL;
	}
	return dev;
}

static void playback_linux_remove(struct device_t * dev)
{
	struct audio_playback_t * playback = (struct audio_playback_t *)dev->priv;
	struct playback_linux_pdata_t * pdat = (struct playback_linux_pdata_t *)playback->priv;

	if(playback)
	{
		unregister_audio_playback(playback);
		if(pdat->ctx)
			linux_audio_playback_stop(pdat->ctx);
		free_device_name(playback->name);
		xos_mem_free(playback->priv);
		xos_mem_free(playback);
	}
}

static void playback_linux_suspend(struct device_t * dev)
{
}

static void playback_linux_resume(struct device_t * dev)
{
}

static struct driver_t playback_linux = {
	.name		= "playback-linux",
	.probe		= playback_linux_probe,
	.remove		= playback_linux_remove,
	.suspend	= playback_linux_suspend,
	.resume		= playback_linux_resume,
};

static void playback_linux_driver_init(void)
{
	register_driver(&playback_linux);
}

static void playback_linux_driver_exit(void)
{
	unregister_driver(&playback_linux);
}

driver_initcall(playback_linux_driver_init);
driver_exitcall(playback_linux_driver_exit);
