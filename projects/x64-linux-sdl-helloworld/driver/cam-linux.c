#include <linux/linux.h>

struct cam_linux_pdata_t {
	char * path;
	void * ctx;

	enum camera_format_t fmt;
	int width;
	int height;
};

static int cam_start(struct camera_t * cam, enum camera_format_t fmt, int width, int height)
{
	struct cam_linux_pdata_t * pdat = (struct cam_linux_pdata_t *)cam->priv;

	pdat->fmt = fmt;
	pdat->width = width;
	pdat->height = height;
	if((pdat->ctx = linux_cam_start(pdat->path, (int *)&pdat->fmt, &pdat->width, &pdat->height)))
		return 1;
	return 0;
}

static int cam_stop(struct camera_t * cam)
{
	struct cam_linux_pdata_t * pdat = (struct cam_linux_pdata_t *)cam->priv;
	if(pdat->ctx)
		linux_cam_stop(pdat->ctx);
	return 1;
}

static int cam_capture(struct camera_t * cam, struct camera_frame_t * frame)
{
	struct cam_linux_pdata_t * pdat = (struct cam_linux_pdata_t *)cam->priv;
	if(pdat->ctx)
	{
		frame->buflen = linux_cam_capture(pdat->ctx, &frame->buf);
		if(frame->buflen > 0)
		{
			frame->fmt = pdat->fmt;
			frame->width = pdat->width;
			frame->height = pdat->height;
			return 1;
		}
	}
	return 0;
}

static int cam_ioctl(struct camera_t * cam, const char * cmd, void * arg)
{
	switch(shash(cmd))
	{
	case 0x440a6553: /* "camera-set-gain" */
		break;
	case 0x30f46ac7: /* "camera-get-gain" */
		break;
	case 0x8531e7af: /* "camera-set-exposure" */
		break;
	case 0x39c98723: /* "camera-get-exposure" */
		break;
	case 0x2d64af48: /* "camera-set-white-balance" */
		break;
	case 0x4b2e173c: /* "camera-get-white-balance" */
		break;
	case 0xc54c8c54: /* "camera-set-focus" */
		break;
	case 0x4f774048: /* "camera-get-focus" */
		break;
	case 0x7ecdcd0f: /* "camera-set-mirror" */
		break;
	case 0x4e4eff83: /* "camera-get-mirror" */
		break;
	case 0xd5d73dfe: /* "camera-set-saturation" */
		break;
	case 0x0ed48a72: /* "camera-get-saturation" */
		break;
	case 0xdae4842d: /* "camera-set-brightness" */
		break;
	case 0x13e1d0a1: /* "camera-get-brightness" */
		break;
	case 0xf3916322: /* "camera-set-contrast" */
		break;
	case 0xa8290296: /* "camera-get-contrast" */
		break;
	case 0x7e2ee316: /* "camera-set-hue" */
		break;
	case 0xe2740a0a: /* "camera-get-hue" */
		break;
	case 0x4a3b52eb: /* "camera-set-sharpness" */
		break;
	case 0x91c6e0df: /* "camera-get-sharpness" */
		break;
	default:
		break;
	}
	return -1;
}

static struct device_t * cam_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct cam_linux_pdata_t * pdat;
	struct camera_t * cam;
	struct device_t * dev;
	char * path = dt_read_string(n, "device", NULL);

	if(!linux_cam_exist(path))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct cam_linux_pdata_t));
	if(!pdat)
		return NULL;

	cam = xos_mem_malloc(sizeof(struct camera_t));
	if(!cam)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->path = xos_strdup(path);
	pdat->ctx = NULL;

	cam->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	cam->start = cam_start;
	cam->stop = cam_stop;
	cam->capture = cam_capture;
	cam->ioctl = cam_ioctl;
	cam->priv = pdat;

	if(!(dev = register_camera(cam, drv)))
	{
		xos_mem_free(pdat->path);
		free_device_name(cam->name);
		xos_mem_free(cam->priv);
		xos_mem_free(cam);
		return NULL;
	}
	return dev;
}

static void cam_linux_remove(struct device_t * dev)
{
	struct camera_t * cam = (struct camera_t *)dev->priv;
	struct cam_linux_pdata_t * pdat = (struct cam_linux_pdata_t *)cam->priv;

	if(cam)
	{
		unregister_camera(cam);
		xos_mem_free(pdat->path);
		free_device_name(cam->name);
		xos_mem_free(cam->priv);
		xos_mem_free(cam);
	}
}

static void cam_linux_suspend(struct device_t * dev)
{
}

static void cam_linux_resume(struct device_t * dev)
{
}

static struct driver_t cam_linux = {
	.name		= "cam-linux",
	.probe		= cam_linux_probe,
	.remove		= cam_linux_remove,
	.suspend	= cam_linux_suspend,
	.resume		= cam_linux_resume,
};

static void cam_linux_driver_init(void)
{
	register_driver(&cam_linux);
}

static void cam_linux_driver_exit(void)
{
	unregister_driver(&cam_linux);
}

driver_initcall(cam_linux_driver_init);
driver_exitcall(cam_linux_driver_exit);
