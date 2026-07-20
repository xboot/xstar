#include <win/win.h>

struct fb_win_sdl_pdata_t {
	int width;
	int height;
	int pwidth;
	int pheight;
	void * priv;
};

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_win_sdl_pdata_t * pdat = (struct fb_win_sdl_pdata_t *)fb->priv;
	win_fb_sdl_set_backlight(pdat->priv, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_win_sdl_pdata_t * pdat = (struct fb_win_sdl_pdata_t *)fb->priv;
	return win_fb_sdl_get_backlight(pdat->priv);
}

static struct surface_t * fb_create(struct framebuffer_t * fb, int width, int height)
{
	struct fb_win_sdl_pdata_t * pdat = (struct fb_win_sdl_pdata_t *)fb->priv;
	struct win_fb_surface_t * surface;
	struct surface_t * s;

	surface = xos_mem_malloc(sizeof(struct win_fb_surface_t));
	if(!surface)
		return NULL;

	if(!win_fb_sdl_surface_create(pdat->priv, surface, width, height))
	{
		xos_mem_free(surface);
		return NULL;
	}

	s = xos_mem_malloc(sizeof(struct surface_t));
	if(!s)
	{
		win_fb_sdl_surface_destroy(pdat->priv, surface);
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
	struct fb_win_sdl_pdata_t * pdat = (struct fb_win_sdl_pdata_t *)fb->priv;

	if(s)
	{
		if(s->cg)
		{
			cg_destroy(s->cg->ctx);
			cg_surface_destroy(s->cg->surface);
			xos_mem_free(s->cg);
		}
		win_fb_sdl_surface_destroy(pdat->priv, s->priv);
		xos_mem_free(s->priv);
		xos_mem_free(s);
	}
}

static void fb_present(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l)
{
	struct fb_win_sdl_pdata_t * pdat = (struct fb_win_sdl_pdata_t *)fb->priv;
	win_fb_sdl_surface_present(pdat->priv, s->priv, (struct win_dirtylist_t *)l);
}

static struct device_t * fb_win_sdl_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_win_sdl_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	int width = dt_read_int(n, "width", 640);
	int height = dt_read_int(n, "height", 480);
	char title[64];
	void * ctx;

	xos_sprintf(title, "Xstar Runtime Environment");
	ctx = win_fb_sdl_open(title, width, height);
	if(!ctx)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_win_sdl_pdata_t));
	if(!pdat)
		return NULL;

	fb = xos_mem_malloc(sizeof(struct framebuffer_t));
	if(!fb)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->priv = ctx;
	pdat->width = win_fb_sdl_get_width(pdat->priv);
	pdat->height = win_fb_sdl_get_height(pdat->priv);
	pdat->pwidth = dt_read_int(n, "physical-width", win_fb_sdl_get_pwidth(pdat->priv));
	pdat->pheight = dt_read_int(n, "physical-height", win_fb_sdl_get_pheight(pdat->priv));

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
		win_fb_sdl_close(pdat->priv);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
		return NULL;
	}
	return dev;
}

static void fb_win_sdl_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_win_sdl_pdata_t * pdat = (struct fb_win_sdl_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		win_fb_sdl_close(pdat->priv);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_win_sdl_suspend(struct device_t * dev)
{
}

static void fb_win_sdl_resume(struct device_t * dev)
{
}

static struct driver_t fb_win_sdl = {
	.name		= "fb-win-sdl",
	.probe		= fb_win_sdl_probe,
	.remove		= fb_win_sdl_remove,
	.suspend	= fb_win_sdl_suspend,
	.resume		= fb_win_sdl_resume,
};

static void fb_win_sdl_driver_init(void)
{
	register_driver(&fb_win_sdl);
}

static void fb_win_sdl_driver_exit(void)
{
	unregister_driver(&fb_win_sdl);
}

driver_initcall(fb_win_sdl_driver_init);
driver_exitcall(fb_win_sdl_driver_exit);
