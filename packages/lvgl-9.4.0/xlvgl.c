/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <xstar.h>
#include <xlvgl.h>

static void xlvgl_disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
	struct xlvgl_context_t * ctx = (struct xlvgl_context_t *)lv_display_get_driver_data(disp);

	window_dirtylist_clear(ctx->win);
	window_dirtylist_add(ctx->win, &(struct region_t){ area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area) });
	window_present_commit(ctx->win);

	lv_display_flush_ready(disp);
}

static uint32_t xlvgl_tick_get(void)
{
	return ktime_to_ms(ktime_get());
}

static void xlvgl_delay_cb(uint32_t ms)
{
	mdelay(ms);
}

static void xlvgl_sleep_cb(uint32_t ms)
{
	xos_thread_sleep(ms * 1000000ULL);
}

static void xlvgl_event_handler(lv_timer_t * timer)
{
	struct xlvgl_context_t * ctx = (struct xlvgl_context_t *)lv_timer_get_user_data(timer);
	struct event_t e;

	while(window_pump_event(ctx->win, &e))
	{
		if(ctx->event)
			ctx->event(&e, ctx->data);
		switch(e.type)
		{
		case EVENT_TYPE_KEY_DOWN:
			ctx->keyboard.key = e.e.key_down.key;
			ctx->keyboard.pressed = 1;
			lv_indev_read(ctx->keyboard.indev);
			break;
		case EVENT_TYPE_KEY_UP:
			ctx->keyboard.key = e.e.key_up.key;
			ctx->keyboard.pressed = 0;
			lv_indev_read(ctx->keyboard.indev);
			break;

		case EVENT_TYPE_ROTARY_STEP:
			ctx->encoder.delta = -e.e.rotary_step.delta;
			lv_indev_read(ctx->encoder.indev);
			break;

		case EVENT_TYPE_MOUSE_DOWN:
			if(e.e.mouse_down.button & MOUSE_BUTTON_LEFT)
			{
				ctx->mouse.pressed = 1;
				lv_indev_read(ctx->mouse.indev);
			}
			break;
		case EVENT_TYPE_MOUSE_MOVE:
			ctx->mouse.lastx = e.e.mouse_move.x;
			ctx->mouse.lasty = e.e.mouse_move.y;
			lv_indev_read(ctx->mouse.indev);
			break;
		case EVENT_TYPE_MOUSE_UP:
			if(e.e.mouse_up.button & MOUSE_BUTTON_LEFT)
			{
				ctx->mouse.pressed = 0;
				lv_indev_read(ctx->mouse.indev);
			}
			break;
		case EVENT_TYPE_MOUSE_WHEEL:
			ctx->encoder.delta = -e.e.mouse_wheel.dy;
			lv_indev_read(ctx->encoder.indev);
			break;

		case EVENT_TYPE_TOUCH_BEGIN:
			ctx->touchscreen.pressed = 1;
			ctx->touchscreen.x = e.e.touch_begin.x;
			ctx->touchscreen.y = e.e.touch_begin.y;
			lv_indev_read(ctx->touchscreen.indev);
			break;
		case EVENT_TYPE_TOUCH_MOVE:
			ctx->touchscreen.x = e.e.touch_move.x;
			ctx->touchscreen.y = e.e.touch_move.y;
			lv_indev_read(ctx->touchscreen.indev);
			break;
		case EVENT_TYPE_TOUCH_END:
			ctx->touchscreen.pressed = 0;
			ctx->touchscreen.x = e.e.touch_end.x;
			ctx->touchscreen.y = e.e.touch_end.y;
			lv_indev_read(ctx->touchscreen.indev);
			break;

		default:
			break;
		}
	}
}

static void xlvgl_keyboard_read(lv_indev_t * indev, lv_indev_data_t * data)
{
	struct xlvgl_context_t * ctx = (struct xlvgl_context_t *)lv_indev_get_driver_data(indev);

	switch(ctx->keyboard.key)
	{
	case KB_KEY_UP:
		data->key = LV_KEY_UP;
		break;
	case KB_KEY_DOWN:
		data->key = LV_KEY_DOWN;
		break;
	case KB_KEY_LEFT:
		data->key = LV_KEY_LEFT;
		break;
	case KB_KEY_RIGHT:
		data->key = LV_KEY_RIGHT;
		break;
	case KB_KEY_VOLUME_UP:
		data->key = LV_KEY_PREV;
		break;
	case KB_KEY_VOLUME_DOWN:
		data->key = LV_KEY_NEXT;
		break;
	case KB_KEY_HOME:
		data->key = LV_KEY_HOME;
		break;
	case KB_KEY_BACK:
		data->key = LV_KEY_BACKSPACE;
		break;
	case KB_KEY_MENU:
		data->key = LV_KEY_ESC;
		break;
	case KB_KEY_ENTER:
		data->key = LV_KEY_ENTER;
		break;
	default:
		data->key = ctx->keyboard.key;
		break;
	}
	data->state = ctx->keyboard.pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void xlvgl_mouse_read(lv_indev_t * indev, lv_indev_data_t * data)
{
	struct xlvgl_context_t * ctx = (struct xlvgl_context_t *)lv_indev_get_driver_data(indev);

	data->point.x = ctx->mouse.lastx;
	data->point.y = ctx->mouse.lasty;
	data->state = ctx->mouse.pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void xlvgl_touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data)
{
	struct xlvgl_context_t * ctx = (struct xlvgl_context_t *)lv_indev_get_driver_data(indev);

	data->point.x = ctx->touchscreen.x;
	data->point.y = ctx->touchscreen.y;
	data->state = ctx->touchscreen.pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void xlvgl_encoder_read(lv_indev_t * indev, lv_indev_data_t * data)
{
	struct xlvgl_context_t * ctx = (struct xlvgl_context_t *)lv_indev_get_driver_data(indev);

	data->enc_diff = ctx->encoder.delta;
	data->state = LV_INDEV_STATE_RELEASED;
}

struct xlvgl_context_t * xlvgl_context_alloc(const char * fb, const char * input, int orientation)
{
	struct xlvgl_context_t * ctx;

	lv_init();
	ctx = xos_mem_malloc(sizeof(struct xlvgl_context_t));
	if(!ctx)
		return NULL;

	xos_memset(ctx, 0, sizeof(struct xlvgl_context_t));
	ctx->win = window_alloc(fb, input, orientation);

	ctx->disp = lv_display_create(window_get_width(ctx->win), window_get_height(ctx->win));
	lv_display_set_default(ctx->disp);
	lv_display_set_driver_data(ctx->disp, ctx);
	lv_display_set_dpi(ctx->disp, window_get_dpi(ctx->win));
	lv_display_set_flush_cb(ctx->disp, xlvgl_disp_flush);
	lv_display_set_buffers(ctx->disp, ctx->win->surface->pixels, NULL, window_get_width(ctx->win) * window_get_height(ctx->win) * 4, LV_DISPLAY_RENDER_MODE_DIRECT);

	lv_tick_set_cb(xlvgl_tick_get);
	lv_delay_set_cb(xstar_feature_thread() ? xlvgl_sleep_cb : xlvgl_delay_cb);
	ctx->timer = lv_timer_create(xlvgl_event_handler, 5, ctx);

	ctx->keyboard.indev = lv_indev_create();
	lv_indev_set_type(ctx->keyboard.indev, LV_INDEV_TYPE_KEYPAD);
	lv_indev_set_read_cb(ctx->keyboard.indev, xlvgl_keyboard_read);
	lv_indev_set_driver_data(ctx->keyboard.indev, ctx);
	lv_indev_set_mode(ctx->keyboard.indev, LV_INDEV_MODE_EVENT);

	ctx->mouse.indev = lv_indev_create();
	lv_indev_set_type(ctx->mouse.indev, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(ctx->mouse.indev, xlvgl_mouse_read);
	lv_indev_set_driver_data(ctx->mouse.indev, ctx);
	lv_indev_set_mode(ctx->mouse.indev, LV_INDEV_MODE_EVENT);

	ctx->touchscreen.indev = lv_indev_create();
	lv_indev_set_type(ctx->touchscreen.indev, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(ctx->touchscreen.indev, xlvgl_touchscreen_read);
	lv_indev_set_driver_data(ctx->touchscreen.indev, ctx);
	lv_indev_set_mode(ctx->touchscreen.indev, LV_INDEV_MODE_EVENT);

	ctx->encoder.indev = lv_indev_create();
	lv_indev_set_type(ctx->encoder.indev, LV_INDEV_TYPE_ENCODER);
	lv_indev_set_read_cb(ctx->encoder.indev, xlvgl_encoder_read);
	lv_indev_set_driver_data(ctx->encoder.indev, ctx);
	lv_indev_set_mode(ctx->encoder.indev, LV_INDEV_MODE_EVENT);

	ctx->event = NULL;
	ctx->data = NULL;

	return ctx;
}

void xlvgl_context_free(struct xlvgl_context_t * ctx)
{
	if(ctx)
	{
		if(ctx->encoder.indev)
			lv_indev_delete(ctx->encoder.indev);
		if(ctx->touchscreen.indev)
			lv_indev_delete(ctx->touchscreen.indev);
		if(ctx->mouse.indev)
			lv_indev_delete(ctx->mouse.indev);
		if(ctx->keyboard.indev)
			lv_indev_delete(ctx->keyboard.indev);
		if(ctx->timer)
			lv_timer_delete(ctx->timer);
		if(ctx->disp)
			lv_display_delete(ctx->disp);
		if(ctx->win)
			window_free(ctx->win);
		xos_mem_free(ctx);
	}
	lv_deinit();
}

void xlvgl_context_step(struct xlvgl_context_t * ctx)
{
    uint32_t next = lv_timer_handler();
    lv_delay_ms(next);
}

void xlvgl_context_hook_event(struct xlvgl_context_t * ctx, void (*event)(struct event_t *, void *), void * data)
{
	if(ctx)
	{
		ctx->event = event;
		ctx->data = data;
	}
}
