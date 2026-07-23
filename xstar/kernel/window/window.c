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

#include <driver/g2d/g2d.h>
#include <driver/clocksource/clocksource.h>
#include <kernel/window/window.h>

static void fb_dummy_setbl(struct framebuffer_t * fb, int brightness)
{
}

static int fb_dummy_getbl(struct framebuffer_t * fb)
{
	return 1000;
}

static struct surface_t * fb_dummy_create(struct framebuffer_t * fb, int width, int height)
{
	return surface_alloc(width, height);
}

static void fb_dummy_destroy(struct framebuffer_t * fb, struct surface_t * s)
{
	surface_free(s);
}

static void fb_dummy_present(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l)
{
}

static struct framebuffer_t fb_dummy = {
	.name		= "fb-dummy",
	.width		= 128,
	.height		= 128,
	.pwidth		= 23,
	.pheight	= 23,
	.setbl		= fb_dummy_setbl,
	.getbl		= fb_dummy_getbl,
	.create		= fb_dummy_create,
	.destroy	= fb_dummy_destroy,
	.present	= fb_dummy_present,
	.priv		= NULL,
};

static struct list_head_t __window_list = {
	.next = &__window_list,
	.prev = &__window_list,
};
static struct mutex_t __window_lock;

enum {
	GMFLAG_IDENTITY		= 0,
	GMFLAG_ROTATE_0		= 1,
	GMFLAG_ROTATE_90	= 2,
	GMFLAG_ROTATE_180	= 3,
	GMFLAG_ROTATE_270	= 4,
	GMFLAG_FLIP_H		= 5,
	GMFLAG_FLIP_MD		= 6,
	GMFLAG_FLIP_V		= 7,
	GMFLAG_FLIP_AD		= 8,
	GMFLAG_OTHER		= 9,
};

static inline int feq(float a, float b)
{
	return fabsf(a - b) < 1e-6f;
}

static inline void window_surface_blit_rotate_0(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx, ty, sw, sh);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = r.x - tx;
		int sy0 = r.y - ty;
		for(int y = 0; y < r.h; y++)
			xos_memcpy(dp + (r.y + y) * ds + r.x, sp + (sy0 + y) * ss + sx0, r.w * 4);
	}
}

static inline void window_surface_blit_rotate_90(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx, ty - sw, sh, sw);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = ty - r.y;
		int sy0 = r.x - tx;
		for(int y = 0; y < r.h; y++)
		{
			uint32_t * d = dp + (r.y + y) * ds + r.x;
			int sx = sx0 - y;
			int sy = sy0;
			for(int x = 0; x < r.w; x++)
				*d++ = *(sp + (sy + x) * ss + sx);
		}
	}
}

static inline void window_surface_blit_rotate_180(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx - sw, ty - sh, sw, sh);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = tx - r.x;
		int sy0 = ty - r.y;
		for(int y = 0; y < r.h; y++)
		{
			uint32_t * d = dp + (r.y + y) * ds + r.x;
			int sx = sx0;
			int sy = sy0 - y;
			for(int x = 0; x < r.w; x++)
				*d++ = *(sp + sy * ss + sx--);
		}
	}
}

static inline void window_surface_blit_rotate_270(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx - sh, ty, sh, sw);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = r.y - ty;
		int sy0 = tx - r.x;
		for(int y = 0; y < r.h; y++)
		{
			uint32_t * d = dp + (r.y + y) * ds + r.x;
			int sx = sx0 + y;
			int sy = sy0;
			for(int x = 0; x < r.w; x++)
				*d++ = *(sp + (sy - x) * ss + sx);
		}
	}
}

static inline void window_surface_blit_flip_h(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx - sw, ty, sw, sh);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = tx - r.x;
		int sy0 = r.y - ty;
		for(int y = 0; y < r.h; y++)
		{
			uint32_t * d = dp + (r.y + y) * ds + r.x;
			int sx = sx0;
			int sy = sy0 + y;
			for(int x = 0; x < r.w; x++)
				*d++ = *(sp + sy * ss + sx--);
		}
	}
}

static inline void window_surface_blit_flip_md(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx, ty, sh, sw);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = r.y - ty;
		int sy0 = r.x - tx;
		for(int y = 0; y < r.h; y++)
		{
			uint32_t * d = dp + (r.y + y) * ds + r.x;
			int sx = sx0 + y;
			int sy = sy0;
			for(int x = 0; x < r.w; x++)
				*d++ = *(sp + (sy + x) * ss + sx);
		}
	}
}

static inline void window_surface_blit_flip_v(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx, ty - sh, sw, sh);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = r.x - tx;
		int sy0 = ty - r.y;
		for(int y = 0; y < r.h; y++)
			xos_memcpy(dp + (r.y + y) * ds + r.x, sp + (sy0 - y) * ss + sx0, r.w * 4);
	}
}

static inline void window_surface_blit_flip_ad(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);
		int tx = (int)m->tx;
		int ty = (int)m->ty;

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		region_init(&region, tx - sh, ty - sw, sh, sw);
		if(!region_intersect(&r, &r, &region))
			return;

		int sx0 = ty - r.y;
		int sy0 = tx - r.x;
		for(int y = 0; y < r.h; y++)
		{
			uint32_t * d = dp + (r.y + y) * ds + r.x;
			int sx = sx0 - y;
			int sy = sy0;
			for(int x = 0; x < r.w; x++)
				*d++ = *(sp + (sy - x) * ss + sx);
		}
	}
}

static inline void window_surface_blit_other(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o)
{
	if(!g2d_blit(s->g2d, s, clip, m, o))
	{
		struct region_t r, region;
		struct matrix2d_t t;
		uint32_t * dp = surface_get_pixels(s);
		uint32_t * sp = surface_get_pixels(o);
		int ds = surface_get_stride(s) >> 2;
		int ss = surface_get_stride(o) >> 2;
		int sw = surface_get_width(o);
		int sh = surface_get_height(o);

		region_init(&r, 0, 0, surface_get_width(s), surface_get_height(s));
		if(clip)
		{
			if(!region_intersect(&r, &r, clip))
				return;
		}
		matrix2d_transform_region(m, sw, sh, &region);
		if(!region_intersect(&r, &r, &region))
			return;

		int x1 = r.x;
		int y1 = r.y;
		int x2 = r.x + r.w;
		int y2 = r.y + r.h;
		int stride = ds - r.w;
		uint32_t * p = dp + y1 * ds + x1;
		float fx = x1;
		float fy = y1;
		xos_memcpy(&t, m, sizeof(struct matrix2d_t));
		matrix2d_invert(&t);
		matrix2d_transform_point(&t, &fx, &fy);

		for(int y = y1; y < y2; ++y, fx += t.c, fy += t.d)
		{
			float ofx = fx;
			float ofy = fy;
			for(int x = x1; x < x2; ++x, ofx += t.a, ofy += t.b)
			{
				int x0 = (int)ofx;
				int y0 = (int)ofy;
				if(x0 >= -1 && x0 < sw && y0 >= -1 && y0 < sh)
				{
					int fx_i = (int)((ofx - x0) * 256);
					int fy_i = (int)((ofy - y0) * 256);
					if(fx_i < 0) fx_i = 0; else if(fx_i > 255) fx_i = 255;
					if(fy_i < 0) fy_i = 0; else if(fy_i > 255) fy_i = 255;
					int x0c = x0 < 0 ? 0 : (x0 >= sw ? sw - 1 : x0);
					int y0c = y0 < 0 ? 0 : (y0 >= sh ? sh - 1 : y0);
					int x1c = x0 + 1 < 0 ? 0 : (x0 + 1 >= sw ? sw - 1 : x0 + 1);
					int y1c = y0 + 1 < 0 ? 0 : (y0 + 1 >= sh ? sh - 1 : y0 + 1);
					uint32_t p00 = *(sp + y0c * ss + x0c);
					uint32_t p10 = *(sp + y0c * ss + x1c);
					uint32_t p01 = *(sp + y1c * ss + x0c);
					uint32_t p11 = *(sp + y1c * ss + x1c);
					int w00 = (256 - fx_i) * (256 - fy_i);
					int w10 = fx_i * (256 - fy_i);
					int w01 = (256 - fx_i) * fy_i;
					int w11 = fx_i * fy_i;
					int ra = ((p00 >> 16) & 0xff) * w00 + ((p10 >> 16) & 0xff) * w10 + ((p01 >> 16) & 0xff) * w01 + ((p11 >> 16) & 0xff) * w11;
					int ga = ((p00 >> 8) & 0xff) * w00 + ((p10 >> 8) & 0xff) * w10 + ((p01 >> 8) & 0xff) * w01 + ((p11 >> 8) & 0xff) * w11;
					int ba = ((p00 >> 0) & 0xff) * w00 + ((p10 >> 0) & 0xff) * w10 + ((p01 >> 0) & 0xff) * w01 + ((p11 >> 0) & 0xff) * w11;
					int aa = ((p00 >> 24) & 0xff) * w00 + ((p10 >> 24) & 0xff) * w10 + ((p01 >> 24) & 0xff) * w01 + ((p11 >> 24) & 0xff) * w11;
					*p = ((aa >> 16) << 24) | ((ra >> 16) << 16) | ((ga >> 16) << 8) | (ba >> 16);
				}
				p++;
			}
			p += stride;
		}
	}
}

struct window_t * window_alloc(const char * fb, const char * input, int orientation)
{
	struct window_t * w;
	struct framebuffer_t * fbdev;
	struct input_t * indev;
	struct device_t * pos, * n;
	char * r, * p;
	int range[2];

	if((orientation < 0) || (orientation > 7))
		orientation = CONFIG_XSTAR_WINDOW_ORIENTATION & 0x7;

	w = xos_mem_malloc(sizeof(struct window_t));
	if(!w)
		return NULL;

	fbdev = search_framebuffer(fb);
	if(!fbdev)
	{
		fbdev = search_first_framebuffer();
		if(!fbdev)
			fbdev = &fb_dummy;
	}

	w->fb = fbdev;
	w->dpi = framebuffer_get_dpi(w->fb);
	w->fbsurface = framebuffer_create_surface(w->fb, framebuffer_get_width(w->fb), framebuffer_get_height(w->fb));
	if(orientation & 0x1)
		w->surface = framebuffer_create_surface(w->fb, framebuffer_get_height(w->fb), framebuffer_get_width(w->fb));
	else
		w->surface = framebuffer_create_surface(w->fb, framebuffer_get_width(w->fb), framebuffer_get_height(w->fb));
	switch(orientation)
	{
	case WINDOW_ORIENTATION_ROTATE_0:
		matrix2d_init(&w->lmatrix, 1, 0, 0, 1, 0, 0);
		break;
	case WINDOW_ORIENTATION_ROTATE_90:
		matrix2d_init(&w->lmatrix, 0, -1, 1, 0, 0, framebuffer_get_height(w->fb));
		break;
	case WINDOW_ORIENTATION_ROTATE_180:
		matrix2d_init(&w->lmatrix, -1, 0, 0, -1, framebuffer_get_width(w->fb), framebuffer_get_height(w->fb));
		break;
	case WINDOW_ORIENTATION_ROTATE_270:
		matrix2d_init(&w->lmatrix, 0, 1, -1, 0, framebuffer_get_width(w->fb), 0);
		break;
	case WINDOW_ORIENTATION_FLIP_H:
		matrix2d_init(&w->lmatrix, -1, 0, 0, 1, framebuffer_get_width(w->fb), 0);
		break;
	case WINDOW_ORIENTATION_FLIP_MD:
		matrix2d_init(&w->lmatrix, 0, 1, 1, 0, 0, 0);
		break;
	case WINDOW_ORIENTATION_FLIP_V:
		matrix2d_init(&w->lmatrix, 1, 0, 0, -1, 0, framebuffer_get_height(w->fb));
		break;
	case WINDOW_ORIENTATION_FLIP_AD:
		matrix2d_init(&w->lmatrix, 0, -1, -1, 0, framebuffer_get_width(w->fb), framebuffer_get_height(w->fb));
		break;
	default:
		break;
	}
	window_set_matrix(w, &(struct matrix2d_t){ 1, 0, 0, 1, 0, 0 });

	w->dirtylist = dirtylist_alloc(0);
	w->event = fifo_alloc(sizeof(struct event_t) * 64);
	if(input)
	{
		w->map = hmap_alloc(0, NULL);
		p = xos_strdup(input);
		if(p && (*p != '\0'))
		{
			while((r = xos_strsep(&p, ",;:|")) != NULL)
			{
				indev = search_input(r);
				if(indev)
				{
					hmap_add(w->map, r, indev);
					if(input_ioctl(indev, "mouse-get-range", &range[0]) >= 0)
					{
						range[0] = framebuffer_get_width(w->fb);
						range[1] = framebuffer_get_height(w->fb);
						input_ioctl(indev, "mouse-set-range", &range[0]);
					}
				}
			}
		}
		xos_mem_free(p);
	}
	else
	{
		w->map = NULL;
		list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_INPUT], head)
		{
			if(input_ioctl(pos->priv, "mouse-get-range", &range[0]) >= 0)
			{
				range[0] = framebuffer_get_width(w->fb);
				range[1] = framebuffer_get_height(w->fb);
				input_ioctl(pos->priv, "mouse-set-range", &range[0]);
			}
		}
	}
	w->copyright = xos_copyright_verify() ? 1 : 0;

	xos_mutex_lock(&__window_lock);
	list_add(&w->list, &__window_list);
	xos_mutex_unlock(&__window_lock);

	return w;
}

void window_free(struct window_t * w)
{
	if(w)
	{
		xos_mutex_lock(&__window_lock);
		list_del(&w->list);
		xos_mutex_unlock(&__window_lock);
		if(w->fbsurface)
			framebuffer_destroy_surface(w->fb, w->fbsurface);
		if(w->surface)
			framebuffer_destroy_surface(w->fb, w->surface);
		dirtylist_free(w->dirtylist);
		fifo_free(w->event);
		hmap_free(w->map);
		xos_mem_free(w);
	}
}

void window_set_matrix(struct window_t * w, struct matrix2d_t * m)
{
	if(w && m)
	{
		struct matrix2d_t * gm = &w->gmatrix;
		matrix2d_multiply(gm, m, &w->lmatrix);
		if(feq(gm->a, 1) && feq(gm->b, 0) && feq(gm->c, 0))
		{
			if(feq(gm->d, 1))
			{
				if(feq(gm->tx, 0) && feq(gm->ty, 0))
					w->gmflag = GMFLAG_IDENTITY;
				else
					w->gmflag = GMFLAG_ROTATE_0;
			}
			else if(feq(gm->d, -1))
				w->gmflag = GMFLAG_FLIP_V;
			else
				w->gmflag = GMFLAG_OTHER;
		}
		else if(feq(gm->a, -1) && feq(gm->b, 0) && feq(gm->c, 0))
		{
			if(feq(gm->d, -1))
				w->gmflag = GMFLAG_ROTATE_180;
			else if(feq(gm->d, 1))
				w->gmflag = GMFLAG_FLIP_H;
			else
				w->gmflag = GMFLAG_OTHER;
		}
		else if(feq(gm->a, 0) && feq(gm->d, 0))
		{
			if(feq(gm->b, -1))
			{
				if(feq(gm->c, 1))
					w->gmflag = GMFLAG_ROTATE_90;
				else if(feq(gm->c, -1))
					w->gmflag = GMFLAG_FLIP_AD;
				else
					w->gmflag = GMFLAG_OTHER;
			}
			else if(feq(gm->b, 1))
			{
				if(feq(gm->c, 1))
					w->gmflag = GMFLAG_FLIP_MD;
				else if(feq(gm->c, -1))
					w->gmflag = GMFLAG_ROTATE_270;
				else
					w->gmflag = GMFLAG_OTHER;
			}
			else
				w->gmflag = GMFLAG_OTHER;
		}
		else
			w->gmflag = GMFLAG_OTHER;
	}
}

void window_exit(struct window_t * w)
{
	struct event_t e;

	if(w)
	{
		e.device = &(struct input_t){ "system", NULL, NULL };
		e.type = EVENT_TYPE_SYSTEM_EXIT;
		e.timestamp = ktime_get();
		fifo_put(w->event, (unsigned char *)&e, sizeof(struct event_t));
	}
}

void window_dirtylist_fullscreen(struct window_t * w)
{
	if(w)
	{
		dirtylist_clear(w->dirtylist);
		dirtylist_add(w->dirtylist, &(struct region_t){ 0, 0, surface_get_width(w->surface), surface_get_height(w->surface) });
	}
}

void window_dirtylist_clear(struct window_t * w)
{
	if(w)
		dirtylist_clear(w->dirtylist);
}

void window_dirtylist_add(struct window_t * w, struct region_t * r)
{
	struct region_t region;

	if(w && r)
	{
		if(region_intersect(&region, &(struct region_t){ 0, 0, surface_get_width(w->surface), surface_get_height(w->surface) }, r))
			dirtylist_add(w->dirtylist, &region);
	}
}

void window_present_clear(struct window_t * w)
{
	if(w && (w->dirtylist->count > 0))
	{
		int l = w->surface->stride >> 2;
		for(int i = 0; i < w->dirtylist->count; i++)
		{
			struct region_t * r = &w->dirtylist->items[i].region;
			int x1 = r->x;
			int y1 = r->y;
			int x2 = r->x + r->w;
			int y2 = r->y + r->h;
			uint32_t * p, * q = (uint32_t *)w->surface->pixels + y1 * l + x1;
			int x, y;
			for(y = y1; y < y2; y++, q += l)
			{
				for(x = x1, p = q; x < x2; x++, p++)
				{
					if((x ^ y) & (1 << 3))
						*p = 0xffabb9bd;
					else
						*p = 0xff899598;
				}
			}
		}
	}
}

void window_present_commit(struct window_t * w)
{
	if(w && (w->dirtylist->count > 0))
	{
		if(!w->copyright)
		{
			struct cg_ctx_t * cg = surface_get_cg_ctx(window_get_surface(w));
			int ww = window_get_width(w);
			int wh = window_get_height(w);
			float sw = (float)ww / 256.0f * 0.618f;
			float sh = (float)wh / 256.0f * 0.618f;
			float scale = (sw < sh) ? sw : sh;
			float tx = ((float)ww - scale * 256.0f) * 0.5f;
			float ty = ((float)wh - scale * 256.0f) * 0.5f;
			cg_save(cg);
			cg_identity_matrix(cg);
			cg_translate(cg, tx, ty);
			cg_scale(cg, scale, scale);
			cg_set_source_rgba(cg, 1.0, 0.0, 0.0, 0.5);
			cg_move_to(cg, 127.64, 21.08);
			cg_cubic_to(cg, 127.29, 21.08, 126.94, 21.21, 126.68, 21.47);
			cg_cubic_to(cg, 121.05, 26.94, 90.33, 53.95, 26.25, 60.47);
			cg_cubic_to(cg, 25.57, 60.53, 25.05, 61.07, 25.05, 61.71);
			cg_line_to(cg, 25.05, 102.67);
			cg_cubic_to(cg, 25.05, 223.51, 123.61, 233.13, 127.52, 233.46);
			cg_line_to(cg, 127.75, 233.46);
			cg_cubic_to(cg, 131.65, 233.13, 230.22, 223.51, 230.22, 102.67);
			cg_line_to(cg, 230.22, 61.71);
			cg_arc_to(cg, 1.3, 1.3, 0, 0, 0, 229.02, 60.46);
			cg_cubic_to(cg, 164.94, 53.95, 134.22, 26.94, 128.6, 21.46);
			cg_arc_to(cg, 1.4, 1.4, 0, 0, 0, 127.64, 21.08);
			cg_move_to(cg, 127.64, 73.66);
			cg_cubic_to(cg, 140.33, 73.66, 150.62, 83.3, 150.62, 95.2);
			cg_line_to(cg, 150.62, 106.13);
			cg_line_to(cg, 144.54, 106.13);
			cg_line_to(cg, 144.54, 95.91);
			cg_cubic_to(cg, 144.54, 91.67, 142.78, 87.69, 139.59, 84.7);
			cg_arc_to(cg, 17.4, 17.4, 0, 0, 0, 127.64, 80.06);
			cg_arc_to(cg, 17.4, 17.4, 0, 0, 0, 115.68, 84.7);
			cg_cubic_to(cg, 112.48, 87.7, 110.73, 91.67, 110.73, 95.9);
			cg_line_to(cg, 110.73, 106.13);
			cg_line_to(cg, 104.65, 106.13);
			cg_line_to(cg, 104.65, 95.2);
			cg_cubic_to(cg, 104.65, 83.3, 114.95, 73.66, 127.64, 73.66);
			cg_move_to(cg, 101.59, 109.54);
			cg_line_to(cg, 153.68, 109.54);
			cg_cubic_to(cg, 160.31, 109.54, 165.69, 114.58, 165.69, 120.8);
			cg_line_to(cg, 165.69, 169.63);
			cg_cubic_to(cg, 165.69, 175.85, 160.31, 180.89, 153.68, 180.89);
			cg_line_to(cg, 101.59, 180.89);
			cg_cubic_to(cg, 94.96, 180.89, 89.59, 175.85, 89.59, 169.63);
			cg_line_to(cg, 89.59, 120.8);
			cg_cubic_to(cg, 89.59, 114.58, 94.96, 109.54, 101.59, 109.54);
			cg_move_to(cg, 127.64, 130.96);
			cg_cubic_to(cg, 122.58, 130.96, 118.49, 134.81, 118.49, 139.54);
			cg_arc_to(cg, 8.5, 8.5, 0, 0, 0, 123.54, 147.21);
			cg_line_to(cg, 122.61, 155.89);
			cg_cubic_to(cg, 122.4, 157.8, 124.01, 159.46, 126.05, 159.46);
			cg_line_to(cg, 129.22, 159.46);
			cg_cubic_to(cg, 131.27, 159.46, 132.87, 157.8, 132.66, 155.89);
			cg_line_to(cg, 131.73, 147.21);
			cg_cubic_to(cg, 134.73, 145.81, 136.79, 142.91, 136.79, 139.54);
			cg_cubic_to(cg, 136.79, 134.81, 132.69, 130.96, 127.64, 130.96);
			cg_fill(cg);
			cg_set_source_rgba(cg, 1.0, 0.0, 0.0, 0.85);
			cg_move_to(cg, 127.64, 15.16);
			cg_cubic_to(cg, 127.02, 15.16, 126.39, 15.36, 125.9, 15.76);
			cg_cubic_to(cg, 119.07, 21.35, 107.28, 29.64, 89.83, 37.49);
			cg_cubic_to(cg, 69, 46.85, 45.43, 53.24, 19.72, 56.47);
			cg_arc_to(cg, 2.53, 2.53, 0, 0, 0, 17.44, 58.9);
			cg_line_to(cg, 17.44, 99.03);
			cg_cubic_to(cg, 17.44, 128.51, 22.76, 154.21, 33.26, 175.42);
			cg_arc_to(cg, 117, 117, 0, 0, 0, 68.39, 217.84);
			cg_cubic_to(cg, 94.52, 237.34, 122.09, 241.21, 127.34, 241.79);
			cg_quad_to(cg, 127.64, 241.82, 127.94, 241.79);
			cg_cubic_to(cg, 133.19, 241.21, 160.74, 237.35, 186.88, 217.84);
			cg_arc_to(cg, 117, 117, 0, 0, 0, 222.02, 175.42);
			cg_cubic_to(cg, 232.51, 154.21, 237.83, 128.51, 237.83, 99.02);
			cg_line_to(cg, 237.83, 58.9);
			cg_cubic_to(cg, 237.83, 57.68, 236.85, 56.63, 235.56, 56.47);
			cg_cubic_to(cg, 209.85, 53.24, 186.26, 46.85, 165.44, 37.49);
			cg_cubic_to(cg, 148, 29.64, 136.2, 21.35, 129.37, 15.77);
			cg_cubic_to(cg, 128.87, 15.37, 128.26, 15.17, 127.64, 15.17);
			cg_move_to(cg, 127.64, 254.72);
			cg_quad_to(cg, 126.68, 254.72, 125.74, 254.62);
			cg_cubic_to(cg, 122.34, 254.25, 113.14, 252.96, 101.18, 249.12);
			cg_arc_to(cg, 141, 141, 0, 0, 1, 59.81, 227.94);
			cg_arc_to(cg, 130, 130, 0, 0, 1, 20.76, 180.86);
			cg_cubic_to(cg, 9.42, 157.92, 3.67, 130.4, 3.67, 99.03);
			cg_line_to(cg, 3.67, 58.9);
			cg_cubic_to(cg, 3.67, 55.17, 5.11, 51.58, 7.73, 48.78);
			cg_arc_to(cg, 16.7, 16.7, 0, 0, 1, 17.89, 43.66);
			cg_cubic_to(cg, 42.16, 40.61, 64.36, 34.62, 83.87, 25.84);
			cg_cubic_to(cg, 99.93, 18.62, 110.67, 11.08, 116.85, 6.03);
			cg_arc_to(cg, 17, 17, 0, 0, 1, 127.64, 2.23);
			cg_cubic_to(cg, 131.6, 2.23, 135.43, 3.58, 138.42, 6.03);
			cg_cubic_to(cg, 144.6, 11.08, 155.34, 18.62, 171.4, 25.83);
			cg_cubic_to(cg, 190.91, 34.62, 213.11, 40.61, 237.38, 43.66);
			cg_cubic_to(cg, 241.32, 44.16, 244.93, 45.97, 247.55, 48.78);
			cg_cubic_to(cg, 250.16, 51.58, 251.6, 55.18, 251.6, 58.9);
			cg_line_to(cg, 251.6, 99.02);
			cg_cubic_to(cg, 251.6, 130.39, 245.85, 157.92, 234.5, 180.84);
			cg_arc_to(cg, 130, 130, 0, 0, 1, 195.46, 227.92);
			cg_arc_to(cg, 141, 141, 0, 0, 1, 154.09, 249.1);
			cg_arc_to(cg, 129, 129, 0, 0, 1, 129.53, 254.6);
			cg_quad_to(cg, 128.59, 254.7, 127.63, 254.7);
			cg_fill(cg);
			cg_restore(cg);
		}
		switch(w->gmflag)
		{
		case GMFLAG_IDENTITY:
		{
			framebuffer_present_surface(w->fb, w->surface, w->dirtylist);
			break;
		}
		case GMFLAG_ROTATE_0:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				item->region.x = rx + gm->tx;
				item->region.y = ry + gm->ty;
				window_surface_blit_rotate_0(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_ROTATE_90:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				int rw = item->region.w;
				int rh = item->region.h;
				item->region.x = ry + gm->tx;
				item->region.y = gm->ty - rx - rw;
				item->region.w = rh;
				item->region.h = rw;
				window_surface_blit_rotate_90(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_ROTATE_180:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				int rw = item->region.w;
				int rh = item->region.h;
				item->region.x = gm->tx - rx - rw;
				item->region.y = gm->ty - ry - rh;
				window_surface_blit_rotate_180(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_ROTATE_270:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				int rw = item->region.w;
				int rh = item->region.h;
				item->region.x = gm->tx - ry - rh;
				item->region.y = rx + gm->ty;
				item->region.w = rh;
				item->region.h = rw;
				window_surface_blit_rotate_270(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_FLIP_H:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				int rw = item->region.w;
				item->region.x = gm->tx - rx - rw;
				item->region.y = ry + gm->ty;
				window_surface_blit_flip_h(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_FLIP_MD:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				int rw = item->region.w;
				int rh = item->region.h;
				item->region.x = ry + gm->tx;
				item->region.y = rx + gm->ty;
				item->region.w = rh;
				item->region.h = rw;
				window_surface_blit_flip_md(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_FLIP_V:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				int rh = item->region.h;
				item->region.x = rx + gm->tx;
				item->region.y = gm->ty - ry - rh;
				window_surface_blit_flip_v(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_FLIP_AD:
		{
			struct matrix2d_t * gm = &w->gmatrix;
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				int rx = item->region.x;
				int ry = item->region.y;
				int rw = item->region.w;
				int rh = item->region.h;
				item->region.x = gm->tx - ry - rh;
				item->region.y = gm->ty - rx - rw;
				item->region.w = rh;
				item->region.h = rw;
				window_surface_blit_flip_ad(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		case GMFLAG_OTHER:
		{
			for(int i = 0; i < w->dirtylist->count; i++)
			{
				struct dirtylist_item_t * item = &w->dirtylist->items[i];
				float x1 = item->region.x;
				float y1 = item->region.y;
				float x2 = item->region.x + item->region.w;
				float y2 = item->region.y + item->region.h;
				matrix2d_transform_bounds(&w->gmatrix, &x1, &y1, &x2, &y2);
				item->region.x = x1;
				item->region.y = y1;
				item->region.w = x2 - x1;
				item->region.h = y2 - y1;
				item->area = item->region.w * item->region.h;
				window_surface_blit_other(w->fbsurface, &item->region, &w->gmatrix, w->surface);
			}
			framebuffer_present_surface(w->fb, w->fbsurface, w->dirtylist);
			break;
		}
		default:
			break;
		}
	}
}

static void global_to_local_point(struct window_t * w , int x, int y, int * nx, int * ny)
{
	struct matrix2d_t * m = &w->gmatrix;

	switch(w->gmflag)
	{
	case GMFLAG_IDENTITY:
		*nx = x;
		*ny = y;
		break;
	case GMFLAG_ROTATE_0:
		*nx = (int)(x - m->tx);
		*ny = (int)(y - m->ty);
		break;
	case GMFLAG_ROTATE_90:
		*nx = (int)(m->ty - y);
		*ny = (int)(x - m->tx);
		break;
	case GMFLAG_ROTATE_180:
		*nx = (int)(m->tx - x);
		*ny = (int)(m->ty - y);
		break;
	case GMFLAG_ROTATE_270:
		*nx = (int)(y - m->ty);
		*ny = (int)(m->tx - x);
		break;
	case GMFLAG_FLIP_H:
		*nx = (int)(m->tx - x);
		*ny = (int)(y - m->ty);
		break;
	case GMFLAG_FLIP_MD:
		*nx = (int)(y - m->ty);
		*ny = (int)(x - m->tx);
		break;
	case GMFLAG_FLIP_V:
		*nx = (int)(x - m->tx);
		*ny = (int)(m->ty - y);
		break;
	case GMFLAG_FLIP_AD:
		*nx = (int)(m->ty - y);
		*ny = (int)(m->tx - x);
		break;
	case GMFLAG_OTHER:
	{
		float det = m->a * m->d - m->c * m->b;
		if(det != 0.0)
		{
			*nx = (int)(((x - m->tx) * m->d + (m->ty - y) * m->c) / det);
			*ny = (int)(((y - m->ty) * m->a + (m->tx - x) * m->b) / det);
		}
		else
		{
			*nx = x;
			*ny = y;
		}
		break;
	}
	default:
		break;
	}
}

int window_pump_event(struct window_t * w, struct event_t * e)
{
	if(w && (fifo_get(w->event, (unsigned char *)e, sizeof(struct event_t)) == sizeof(struct event_t)))
	{
		if(w->gmflag != GMFLAG_IDENTITY)
		{
			switch(e->type)
			{
			case EVENT_TYPE_MOUSE_DOWN:
				global_to_local_point(w, e->e.mouse_down.x, e->e.mouse_down.y, &e->e.mouse_down.x, &e->e.mouse_down.y);
				break;
			case EVENT_TYPE_MOUSE_MOVE:
				global_to_local_point(w, e->e.mouse_move.x, e->e.mouse_move.y, &e->e.mouse_move.x, &e->e.mouse_move.y);
				break;
			case EVENT_TYPE_MOUSE_UP:
				global_to_local_point(w, e->e.mouse_up.x, e->e.mouse_up.y, &e->e.mouse_up.x, &e->e.mouse_up.y);
				break;
			case EVENT_TYPE_MOUSE_WHEEL:
				break;

			case EVENT_TYPE_TOUCH_BEGIN:
				global_to_local_point(w, e->e.touch_begin.x, e->e.touch_begin.y, &e->e.touch_begin.x, &e->e.touch_begin.y);
				break;
			case EVENT_TYPE_TOUCH_MOVE:
				global_to_local_point(w, e->e.touch_move.x, e->e.touch_move.y, &e->e.touch_move.x, &e->e.touch_move.y);
				break;
			case EVENT_TYPE_TOUCH_END:
				global_to_local_point(w, e->e.touch_end.x, e->e.touch_end.y, &e->e.touch_end.x, &e->e.touch_end.y);
				break;

			default:
				break;
			}
		}
		return 1;
	}
	return 0;
}

void push_event(struct event_t * e)
{
	struct window_t * pos, * n;

	if(e)
	{
		e->timestamp = ktime_get();
		list_for_each_entry_safe(pos, n, &__window_list, list)
		{
			if(pos && (!pos->map || hmap_search(pos->map, ((struct input_t *)e->device)->name)))
			{
				fifo_put(pos->event, (unsigned char *)e, sizeof(struct event_t));
			}
		}
	}
}

static void window_pure_init(void)
{
	xos_mutex_init(&__window_lock);
}

static void window_pure_exit(void)
{
	xos_mutex_exit(&__window_lock);
}

pure_initcall(window_pure_init);
pure_exitcall(window_pure_exit);
