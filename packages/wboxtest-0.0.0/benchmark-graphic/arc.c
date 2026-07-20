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

struct wbt_arc_pdata_t
{
	struct window_t * w;

	ktime_t t1;
	ktime_t t2;
	int calls;
};

static void * arc_setup(struct wboxtest_t * wbt)
{
	struct wbt_arc_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct wbt_arc_pdata_t));
	if(!pdat)
		return NULL;

	pdat->w = window_alloc(NULL, NULL, -1);
	if(!pdat->w)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	return pdat;
}

static void arc_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_arc_pdata_t * pdat = (struct wbt_arc_pdata_t *)data;

	if(pdat)
	{
		window_free(pdat->w);
		xos_mem_free(pdat);
	}
}

static void arc_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_arc_pdata_t * pdat = (struct wbt_arc_pdata_t *)data;
	struct surface_t * s = window_get_surface(pdat->w);

	if(pdat)
	{
		xos_srand(0);
		pdat->calls = 0;
		pdat->t2 = pdat->t1 = ktime_get();
		do {
			int x = wboxtest_random_int(0, surface_get_width(s));
			int y = wboxtest_random_int(0, surface_get_height(s));
			int radius = wboxtest_random_int(0, XMIN(surface_get_width(s), surface_get_height(s)));
			int a1 = wboxtest_random_int(0, 360);
			int a2 = wboxtest_random_int(0, 360);
			struct color_t c;
			color_init(&c, xos_rand() & 0xff, xos_rand() & 0xff, xos_rand() & 0xff, 255);
			int thickness = wboxtest_random_int(0, 50);
			struct cg_ctx_t * cg = surface_get_cg_ctx(s);
			cg_save(cg);
			cg_arc(cg, x, y, radius, a1 * (M_PI / 180.0), a2 * (M_PI / 180.0));
			cg_set_source_rgba(cg, c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
			if(thickness > 0)
			{
				cg_set_line_width(cg, thickness);
				cg_stroke(cg);
			}
			else
			{
				cg_fill(cg);
			}
			cg_restore(cg);
			pdat->calls++;
			pdat->t2 = ktime_get();
		} while(ktime_before(pdat->t2, ktime_add_ms(pdat->t1, 2000)));
		wboxtest_printf(" Counts: %g\r\n", (double)(pdat->calls * 1000.0) / ktime_ms_delta(pdat->t2, pdat->t1));
	}
}

static struct wboxtest_t wbt_arc = {
	.group	= "benchmark-graphic",
	.name	= "arc",
	.setup	= arc_setup,
	.clean	= arc_clean,
	.run	= arc_run,
};

static void arc_wbt_init(void)
{
	register_wboxtest(&wbt_arc);
}

static void arc_wbt_exit(void)
{
	unregister_wboxtest(&wbt_arc);
}

wboxtest_initcall(arc_wbt_init);
wboxtest_exitcall(arc_wbt_exit);
