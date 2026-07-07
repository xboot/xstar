#ifndef __XSTAR_KERNEL_WINDOW_WINDOW_H__
#define __XSTAR_KERNEL_WINDOW_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/input/input.h>
#include <driver/framebuffer/framebuffer.h>
#include <kernel/graphic/surface.h>
#include <kernel/window/event.h>

/*
 * rotation   : 0-3
 * reflection : 4-7
 */
enum window_orientation_t {
	WINDOW_ORIENTATION_ROTATE_0		= 0,	/*   0 counter-clockwise */
	WINDOW_ORIENTATION_ROTATE_90	= 1,	/*  90 counter-clockwise */
	WINDOW_ORIENTATION_ROTATE_180	= 2,	/* 180 counter-clockwise */
	WINDOW_ORIENTATION_ROTATE_270	= 3,	/* 270 counter-clockwise */
	WINDOW_ORIENTATION_FLIP_H		= 4,	/* reflect across y-axis (left-right) */
	WINDOW_ORIENTATION_FLIP_MD		= 5,	/* reflect across main diagonal (y = x) */
	WINDOW_ORIENTATION_FLIP_V		= 6,	/* reflect across x-axis (top-bottom) */
	WINDOW_ORIENTATION_FLIP_AD		= 7,	/* reflect across anti-diagonal (y = -x) */
};

struct window_t {
	struct list_head_t list;
	struct matrix2d_t lmatrix;
	struct matrix2d_t gmatrix;
	struct framebuffer_t * fb;
	struct surface_t * fbsurface;
	struct surface_t * surface;
	struct dirtylist_t * dirtylist;
	struct fifo_t * event;
	struct hmap_t * map;
	struct {
		struct surface_t * s;
		struct region_t r;
	} watermark;
	int gmflag;
	int dpi;
};

static inline struct surface_t * window_get_surface(struct window_t * w)
{
	if(w)
		return w->surface;
	return NULL;
}

static inline int window_get_width(struct window_t * w)
{
	if(w)
		return surface_get_width(w->surface);
	return 0;
}

static inline int window_get_height(struct window_t * w)
{
	if(w)
		return surface_get_height(w->surface);
	return 0;
}

static inline int window_get_pwidth(struct window_t * w)
{
	if(w)
	{
		if((surface_get_width(w->surface) == framebuffer_get_width(w->fb)) && (surface_get_height(w->surface) == framebuffer_get_height(w->fb)))
			return framebuffer_get_pwidth(w->fb);
		else
			return framebuffer_get_pheight(w->fb);
	}
	return 0;
}

static inline int window_get_pheight(struct window_t * w)
{
	if(w)
	{
		if((surface_get_width(w->surface) == framebuffer_get_width(w->fb)) && (surface_get_height(w->surface) == framebuffer_get_height(w->fb)))
			return framebuffer_get_pheight(w->fb);
		else
			return framebuffer_get_pwidth(w->fb);
	}
	return 0;
}

static inline int window_get_dpi(struct window_t * w)
{
	if(w)
		return w->dpi;
	return 0;
}

static inline int window_dp_to_px(struct window_t * w, int dp)
{
	if(w && (dp > 0))
		return XMAX((int)((w->dpi * dp + 80) / 160), (int)1);
	return 0;
}

static inline void window_set_backlight(struct window_t * w, int brightness)
{
	if(w)
		framebuffer_set_backlight(w->fb, brightness);
}

static inline int window_get_backlight(struct window_t * w)
{
	if(w)
		return framebuffer_get_backlight(w->fb);
	return 0;
}

struct window_t * window_alloc(const char * fb, const char * input, int orientation);
void window_free(struct window_t * w);
void window_set_watermark(struct window_t * w, const void * buf, int len);
void window_set_matrix(struct window_t * w, struct matrix2d_t * m);
void window_exit(struct window_t * w);
void window_dirtylist_fullscreen(struct window_t * w);
void window_dirtylist_clear(struct window_t * w);
void window_dirtylist_add(struct window_t * w, struct region_t * r);
void window_present_clear(struct window_t * w);
void window_present_commit(struct window_t * w);
int window_pump_event(struct window_t * w, struct event_t * e);
void push_event(struct event_t * e);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_WINDOW_WINDOW_H__ */
