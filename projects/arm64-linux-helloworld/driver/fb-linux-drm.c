#include <linux/linux.h>

struct fb_linux_drm_pdata_t {
	int width;
	int height;
	int pwidth;
	int pheight;
	void * priv;
};

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_linux_drm_pdata_t * pdat = (struct fb_linux_drm_pdata_t *)fb->priv;
	linux_fb_drm_set_backlight(pdat->priv, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_linux_drm_pdata_t * pdat = (struct fb_linux_drm_pdata_t *)fb->priv;
	return linux_fb_drm_get_backlight(pdat->priv);
}

static struct surface_t * fb_create(struct framebuffer_t * fb, int width, int height)
{
	struct fb_linux_drm_pdata_t * pdat = (struct fb_linux_drm_pdata_t *)fb->priv;
	struct linux_fb_surface_t * surface;
	struct surface_t * s;

	surface = xos_mem_malloc(sizeof(struct linux_fb_surface_t));
	if(!surface)
		return NULL;

	if(!linux_fb_drm_surface_create(pdat->priv, surface, width, height))
	{
		xos_mem_free(surface);
		return NULL;
	}

	s = xos_mem_malloc(sizeof(struct surface_t));
	if(!s)
	{
		linux_fb_drm_surface_destroy(pdat->priv, surface);
		xos_mem_free(surface);
		return NULL;
	}

	s->width = surface->width;
	s->height = surface->height;
	s->stride = surface->stride;
	s->pixlen = surface->pixlen;
	s->pixels = surface->pixels;
	s->g2d = search_first_g2d();
	s->cg = NULL;
	s->priv = surface;

	return s;
}

static void fb_destroy(struct framebuffer_t * fb, struct surface_t * s)
{
	struct fb_linux_drm_pdata_t * pdat = (struct fb_linux_drm_pdata_t *)fb->priv;

	if(s)
	{
		if(s->cg)
		{
			cg_destroy(s->cg->ctx);
			cg_surface_destroy(s->cg->surface);
			xos_mem_free(s->cg);
		}
		linux_fb_drm_surface_destroy(pdat->priv, s->priv);
		xos_mem_free(s->priv);
		xos_mem_free(s);
	}
}

static void fb_present(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l)
{
	struct fb_linux_drm_pdata_t * pdat = (struct fb_linux_drm_pdata_t *)fb->priv;
	linux_fb_drm_surface_present(pdat->priv, s->priv, (struct linux_dirtylist_t *)l);
}

static struct device_t * fb_linux_drm_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_linux_drm_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	void * ctx;

	ctx = linux_fb_drm_open(dt_read_string(n, "device", NULL), dt_read_string(n, "connector", NULL));
	if(!ctx)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_linux_drm_pdata_t));
	if(!pdat)
		return NULL;

	fb = xos_mem_malloc(sizeof(struct framebuffer_t));
	if(!fb)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->priv = ctx;
	pdat->width = linux_fb_drm_get_width(pdat->priv);
	pdat->height = linux_fb_drm_get_height(pdat->priv);
	pdat->pwidth = dt_read_int(n, "physical-width", linux_fb_drm_get_pwidth(pdat->priv));
	pdat->pheight = dt_read_int(n, "physical-height", linux_fb_drm_get_pheight(pdat->priv));

	fb->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	fb->width = pdat->width;
	fb->height = pdat->height;
	fb->pwidth = pdat->pwidth;
	fb->pheight = pdat->pheight;
	fb->setbl = fb_setbl;
	fb->getbl = fb_getbl;
	fb->create = fb_create;
	fb->destroy = fb_destroy;
	fb->present = fb_present;
	fb->priv = pdat;

	if(!(dev = register_framebuffer(fb, drv)))
	{
		linux_fb_drm_close(pdat->priv);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
		return NULL;
	}
	return dev;
}

static void fb_linux_drm_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_linux_drm_pdata_t * pdat = (struct fb_linux_drm_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		linux_fb_drm_close(pdat->priv);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_linux_drm_suspend(struct device_t * dev)
{
}

static void fb_linux_drm_resume(struct device_t * dev)
{
}

static struct driver_t fb_linux_drm = {
	.name		= "fb-linux-drm",
	.probe		= fb_linux_drm_probe,
	.remove		= fb_linux_drm_remove,
	.suspend	= fb_linux_drm_suspend,
	.resume		= fb_linux_drm_resume,
};

static void fb_linux_drm_driver_init(void)
{
	register_driver(&fb_linux_drm);
}

static void fb_linux_drm_driver_exit(void)
{
	unregister_driver(&fb_linux_drm);
}

driver_initcall(fb_linux_drm_driver_init);
driver_exitcall(fb_linux_drm_driver_exit);
