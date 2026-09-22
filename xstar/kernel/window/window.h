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
	struct surface_t * fbsurface[2];
	struct surface_t * surface[2];
	struct dirtylist_t * pending;
	struct dirtylist_t * inflight;
	struct dirtylist_t * fbspace[2];
	struct mutex_t lock;
	struct fifo_t * event;
	struct hmap_t * map;
	int bufdbl;
	int bufidx;
	int copyright;
	int gmflag;
	int dpi;
};

static inline int window_get_width(struct window_t * w)
{
	if(w)
		return surface_get_width(w->surface[0] ? w->surface[0] : w->fbsurface[0]);
	return 0;
}

static inline int window_get_height(struct window_t * w)
{
	if(w)
		return surface_get_height(w->surface[0] ? w->surface[0] : w->fbsurface[0]);
	return 0;
}

static inline int window_get_pwidth(struct window_t * w)
{
	if(w)
	{
		if((window_get_width(w) == framebuffer_get_width(w->fb)) && (window_get_height(w) == framebuffer_get_height(w->fb)))
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
		if((window_get_width(w) == framebuffer_get_width(w->fb)) && (window_get_height(w) == framebuffer_get_height(w->fb)))
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
void window_set_matrix(struct window_t * w, struct matrix2d_t * m);
void window_exit(struct window_t * w);
struct surface_t * window_frame_acquire(struct window_t * w);
struct dirtylist_t * window_frame_damage_get(struct window_t * w);
void window_frame_damage(struct window_t * w, struct region_t * r);
void window_frame_clear(struct window_t * w);
int window_frame_release(struct window_t * w, void (*cb)(void *), void * data);
void window_frame_wait(struct window_t * w);
int window_pump_event(struct window_t * w, struct event_t * e);
void push_event(struct event_t * e);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_WINDOW_WINDOW_H__ */
