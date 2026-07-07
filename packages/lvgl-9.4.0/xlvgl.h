#ifndef __XSTAR_XLVGL_H__
#define __XSTAR_XLVGL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <kernel/window/window.h>
#include <lvgl/lvgl.h>

struct xlvgl_context_t {
	struct window_t * win;
	lv_display_t * disp;
	lv_timer_t * timer;
	struct {
		lv_indev_t * indev;
		uint32_t key;
		uint8_t pressed;
	} keyboard;
	struct {
		lv_indev_t * indev;
		int32_t lastx;
		int32_t lasty;
		uint8_t pressed;
	} mouse;
	struct {
		lv_indev_t * indev;
		int32_t x;
		int32_t y;
		uint8_t pressed;
	} touchscreen;
	struct {
		lv_indev_t * indev;
		int32_t delta;
	} encoder;
	void (*event)(struct event_t * e, void * data);
	void * data;
};

struct xlvgl_context_t * xlvgl_context_alloc(const char * fb, const char * input, int orientation);
void xlvgl_context_free(struct xlvgl_context_t * ctx);
void xlvgl_context_step(struct xlvgl_context_t * ctx);
void xlvgl_context_hook_event(struct xlvgl_context_t * ctx, void (*event)(struct event_t *, void *), void * data);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_XLVGL_H__ */
