#ifndef __XSTAR_DRIVER_FRAMEBUFFER_H__
#define __XSTAR_DRIVER_FRAMEBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <kernel/graphic/surface.h>

struct framebuffer_t
{
	/* Framebuffer name */
	char * name;

	/* The width and height in pixel */
	int width, height;

	/* The physical size in millimeter */
	int pwidth, pheight;

	/* Set backlight brightness */
	void (*setbl)(struct framebuffer_t * fb, int brightness);

	/* Get backlight brightness */
	int (*getbl)(struct framebuffer_t * fb);

	/* Create a surface */
	struct surface_t * (*create)(struct framebuffer_t * fb, int width, int height);

	/* Destroy a surface */
	void (*destroy)(struct framebuffer_t * fb, struct surface_t * s);

	/* Present a surface */
	void (*present)(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l);

	/* Private data */
	void * priv;
};

static inline void present_surface(void * vram, struct surface_t * s, struct dirtylist_t * l)
{
	struct region_t * r;
	unsigned char * p, * q;
	int count = l->count;
	int stride = s->stride;
	int offset, line, height;
	int i, j;

	for(i = 0; i < count; i++)
	{
		r = &l->items[i].region;
		offset = r->y * stride + (r->x << 2);
		line = r->w << 2;
		height = r->h;

		p = (unsigned char *)vram + offset;
		q = (unsigned char *)s->pixels + offset;
		for(j = 0; j < height; j++, p += stride, q += stride)
			xos_memcpy(p, q, line);
	}
}

static inline int framebuffer_get_width(struct framebuffer_t * fb)
{
	return fb->width;
}

static inline int framebuffer_get_height(struct framebuffer_t * fb)
{
	return fb->height;
}

static inline int framebuffer_get_pwidth(struct framebuffer_t * fb)
{
	return fb->pwidth;
}

static inline int framebuffer_get_pheight(struct framebuffer_t * fb)
{
	return fb->pheight;
}

static inline int framebuffer_get_dpi(struct framebuffer_t * fb)
{
	if((fb->pwidth > 0) && (fb->pheight > 0))
	{
		float p = fb->width * fb->width + fb->height * fb->height;
		float m = fb->pwidth * fb->pwidth + fb->pheight * fb->pheight;
		return (int)(25.4f * sqrtf(p / m));
	}
	return 0;
}

static inline struct surface_t * framebuffer_create_surface(struct framebuffer_t * fb, int width, int height)
{
	return fb->create(fb, width, height);
}

static inline void framebuffer_destroy_surface(struct framebuffer_t * fb, struct surface_t * s)
{
	fb->destroy(fb, s);
}

static inline void framebuffer_present_surface(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l)
{
	fb->present(fb, s, l);
}

struct framebuffer_t * search_framebuffer(const char * name);
struct framebuffer_t * search_first_framebuffer(void);
struct device_t * register_framebuffer(struct framebuffer_t * fb, struct driver_t * drv);
void unregister_framebuffer(struct framebuffer_t * fb);

void framebuffer_set_backlight(struct framebuffer_t * fb, int brightness);
int framebuffer_get_backlight(struct framebuffer_t * fb);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_FRAMEBUFFER_H__ */
