#include <linux/linux.h>

struct linux_fb_context_t {
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;
	int fd;
	int vramsz;
	void * vram;
	int brightness;
};

void * linux_fb_open(const char * dev)
{
	struct linux_fb_context_t * ctx;

	ctx = malloc(sizeof(struct linux_fb_context_t));
	if(!ctx)
		return NULL;
	ctx->fd = open(dev, O_RDWR);
	if(ctx->fd < 0)
	{
		free(ctx);
		return NULL;
	}
	if(ioctl(ctx->fd, FBIOGET_FSCREENINFO, &ctx->fi) != 0)
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}
	if(ioctl(ctx->fd, FBIOGET_VSCREENINFO, &ctx->vi) != 0)
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}
	ctx->vramsz = ctx->vi.yres_virtual * ctx->fi.line_length;
	ctx->vram = mmap(0, ctx->vramsz, PROT_READ | PROT_WRITE, MAP_SHARED, ctx->fd, 0);
	if(ctx->vram == (void *)(-1))
	{
		close(ctx->fd);
		free(ctx);
		return NULL;
	}
	memset(ctx->vram, 0, ctx->vramsz);

	return ctx;
}

void linux_fb_close(void * context)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;

	if(!ctx)
		return;
	if(ctx->vram != (void *)(-1))
		munmap(ctx->vram, ctx->vramsz);
	if(!(ctx->fd < 0))
		close(ctx->fd);
	free(ctx);
}

int linux_fb_get_width(void * context)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;
	if(ctx)
		return ctx->vi.xres;
	return 0;
}

int linux_fb_get_height(void * context)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;
	if(ctx)
		return ctx->vi.yres;
	return 0;
}

int linux_fb_get_pwidth(void * context)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;
	if(ctx)
		return 256;
	return 0;
}

int linux_fb_get_pheight(void * context)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;
	if(ctx)
		return 135;
	return 0;
}

int linux_fb_surface_create(void * context, struct linux_fb_surface_t * surface, int width, int height)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;
	surface->width = width;
	surface->height = height;
	surface->stride = width << 2;
	surface->pixlen = ctx->vramsz;
	surface->pixels = memalign(4, ctx->vramsz);
	return 1;
}

int linux_fb_surface_destroy(void * context, struct linux_fb_surface_t * surface)
{
	if(surface && surface->pixels)
		free(surface->pixels);
	return 1;
}

int linux_fb_surface_present(void * context, struct linux_fb_surface_t * surface, struct linux_dirtylist_t * l)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;

	if(l && (l->count > 0))
	{
		for(int i = 0; i < l->count; i++)
		{
			struct linux_region_t * r = &l->items[i].region;
			uint16_t * q = ctx->vram + r->y * ctx->fi.line_length + r->x * (ctx->vi.bits_per_pixel >> 3);
			for(int y = 0; y < r->h; y++)
			{
				uint32_t * p = surface->pixels + (r->y + y) * surface->stride + (r->x << 2);
				for(int x = 0; x < r->w; x++)
				{
					uint32_t v = *p++;
					*q++ = (((v >> 19) & 0x1f) << 11) | (((v >> 10) & 0x3f) << 5) | ((v >> 3) & 0x1f);
				}
			}
		}
		ioctl(ctx->fd, FBIOPAN_DISPLAY, &ctx->vi);
	}
	return 1;
}

void linux_fb_set_backlight(void * context, int brightness)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;

	if(ctx->brightness != brightness)
	{
		int v = brightness * 255 / 1000;
		char tmp[32];
		xos_sprintf(tmp, "%d", v);
		linux_sysfs_write_string("/sys/class/backlight/lcd_fb0/brightness", tmp);
		ctx->brightness = brightness;
	}
}

int linux_fb_get_backlight(void * context)
{
	struct linux_fb_context_t * ctx = (struct linux_fb_context_t *)context;
	return ctx->brightness;
}
