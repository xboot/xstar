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

#include <wboxtest.h>

struct wbt_preview_pdata_t {
	struct window_t * w;
	struct camera_t * c;
	struct surface_t * s;
	struct camera_frame_t frame;
};

static void * preview_setup(struct wboxtest_t * wbt)
{
	struct wbt_preview_pdata_t * pdat;
	const char * name = NULL;

	pdat = malloc(sizeof(struct wbt_preview_pdata_t));
	if(!pdat)
		return NULL;

	pdat->w = window_alloc(NULL, NULL, -1);
	if(!pdat->w)
	{
		free(pdat);
		return NULL;
	}

	pdat->c = name ? search_camera(name) : search_first_camera();
	if(!pdat->c)
	{
		window_free(pdat->w);
		free(pdat);
		return NULL;
	}

	if(!camera_start(pdat->c, CAMERA_FORMAT_MJPG, 320, 240))
	{
		window_free(pdat->w);
		free(pdat);
		return NULL;
	}

	if(!camera_capture(pdat->c, &pdat->frame, 3000))
	{
		camera_stop(pdat->c);
		window_free(pdat->w);
		free(pdat);
		return NULL;
	}

	pdat->s = surface_alloc(pdat->frame.width, pdat->frame.height);
	if(!pdat->s)
	{
		camera_stop(pdat->c);
		window_free(pdat->w);
		free(pdat);
		return NULL;
	}

	return pdat;
}

static void preview_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_preview_pdata_t * pdat = (struct wbt_preview_pdata_t *)data;

	if(pdat)
	{
		camera_stop(pdat->c);
		surface_free(pdat->s);
		window_free(pdat->w);
		free(pdat);
	}
}

static void preview_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_preview_pdata_t * pdat = (struct wbt_preview_pdata_t *)data;
	int frame = 1200;

	assert_not_null(pdat);
	if(pdat)
	{
		while(frame-- > 0)
		{
			ktime_t timeout = ktime_add_ms(ktime_get(), 16);
			window_dirtylist_fullscreen(pdat->w);
			window_present_clear(pdat->w);
			{
				if(camera_capture(pdat->c, &pdat->frame, 0))
					camera_frame_to_argb(&pdat->frame, pdat->s->pixels);
				struct matrix2d_t m;
				matrix2d_init_identity(&m);
				matrix2d_init_translate(&m, (window_get_width(pdat->w) - surface_get_width(pdat->s)) / 2, (window_get_height(pdat->w) - surface_get_height(pdat->s)) / 2);
				surface_blit(window_get_surface(pdat->w), NULL, &m, pdat->s);
			}
			window_present_commit(pdat->w);
			while(ktime_before(ktime_get(), timeout));
		}
	}
}

static struct wboxtest_t wbt_preview = {
	.group	= "camera",
	.name	= "preview",
	.setup	= preview_setup,
	.clean	= preview_clean,
	.run	= preview_run,
};

static void preview_wbt_init(void)
{
	register_wboxtest(&wbt_preview);
}

static void preview_wbt_exit(void)
{
	unregister_wboxtest(&wbt_preview);
}

wboxtest_initcall(preview_wbt_init);
wboxtest_exitcall(preview_wbt_exit);
