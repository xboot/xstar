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

struct wbt_curve_pdata_t
{
	struct window_t * w;

	ktime_t t1;
	ktime_t t2;
	int calls;
};

static void * curve_setup(struct wboxtest_t * wbt)
{
	struct wbt_curve_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct wbt_curve_pdata_t));
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

static void curve_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_curve_pdata_t * pdat = (struct wbt_curve_pdata_t *)data;

	if(pdat)
	{
		window_free(pdat->w);
		xos_mem_free(pdat);
	}
}

static void curve_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_curve_pdata_t * pdat = (struct wbt_curve_pdata_t *)data;
	struct surface_t * s = window_get_surface(pdat->w);

	if(pdat)
	{
		xos_srand(0);
		pdat->calls = 0;
		pdat->t2 = pdat->t1 = ktime_get();
		do {
			int x[10] = { 0 }, y[10] = { 0 };
			int n = wboxtest_random_int(4, 10);
			for(int i = 0; i < n; i++)
			{
				x[i] = wboxtest_random_int(0, surface_get_width(s));
				y[i] = wboxtest_random_int(0, surface_get_height(s));
			}
			struct color_t c;
			color_init(&c, xos_rand() & 0xff, xos_rand() & 0xff, xos_rand() & 0xff, 255);
			int thickness = wboxtest_random_int(0, 50);
			struct cg_ctx_t * cg = surface_get_cg_ctx(s);
			cg_save(cg);
			cg_move_to(cg, x[0], y[0]);
			for(int i = 1; i <= n - 3; i += 3)
				cg_cubic_to(cg, x[i], y[i], x[i + 1], y[i + 1], x[i + 2], y[i + 2]);
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

static struct wboxtest_t wbt_curve = {
	.group	= "benchmark-graphic",
	.name	= "curve",
	.setup	= curve_setup,
	.clean	= curve_clean,
	.run	= curve_run,
};

static void curve_wbt_init(void)
{
	register_wboxtest(&wbt_curve);
}

static void curve_wbt_exit(void)
{
	unregister_wboxtest(&wbt_curve);
}

wboxtest_initcall(curve_wbt_init);
wboxtest_exitcall(curve_wbt_exit);
