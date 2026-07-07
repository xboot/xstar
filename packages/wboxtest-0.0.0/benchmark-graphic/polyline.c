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

struct wbt_polyline_pdata_t
{
	struct window_t * w;

	ktime_t t1;
	ktime_t t2;
	int calls;
};

static void * polyline_setup(struct wboxtest_t * wbt)
{
	struct wbt_polyline_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct wbt_polyline_pdata_t));
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

static void polyline_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_polyline_pdata_t * pdat = (struct wbt_polyline_pdata_t *)data;

	if(pdat)
	{
		window_free(pdat->w);
		xos_mem_free(pdat);
	}
}

static void polyline_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_polyline_pdata_t * pdat = (struct wbt_polyline_pdata_t *)data;
	struct surface_t * s = window_get_surface(pdat->w);

	if(pdat)
	{
		srand(0);
		pdat->calls = 0;
		pdat->t2 = pdat->t1 = ktime_get();
		do {
			int x[10], y[10];
			int n = wboxtest_random_int(2, 10);
			for(int i = 0; i < n; i++)
			{
				x[i] = wboxtest_random_int(0, surface_get_width(s));
				y[i] = wboxtest_random_int(0, surface_get_height(s));
			}
			struct color_t c;
			color_init(&c, xos_rand() & 0xff, xos_rand() & 0xff, xos_rand() & 0xff, 255);
			int thickness = wboxtest_random_int(0, 50);
			surface_shape_save(s);
			surface_shape_move_to(s, x[0], y[0]);
			for(int i = 1; i < n; i++)
				surface_shape_line_to(s, x[i], y[i]);
			surface_shape_set_source_color(s, &c);
			if(thickness > 0)
			{
				surface_shape_set_line_width(s, thickness);
				surface_shape_stroke(s);
			}
			else
			{
				surface_shape_fill(s);
			}
			surface_shape_restore(s);
			pdat->calls++;
			pdat->t2 = ktime_get();
		} while(ktime_before(pdat->t2, ktime_add_ms(pdat->t1, 2000)));
		wboxtest_printf(" Counts: %g\r\n", (double)(pdat->calls * 1000.0) / ktime_ms_delta(pdat->t2, pdat->t1));
	}
}

static struct wboxtest_t wbt_polyline = {
	.group	= "benchmark-graphic",
	.name	= "polyline",
	.setup	= polyline_setup,
	.clean	= polyline_clean,
	.run	= polyline_run,
};

static void polyline_wbt_init(void)
{
	register_wboxtest(&wbt_polyline);
}

static void polyline_wbt_exit(void)
{
	unregister_wboxtest(&wbt_polyline);
}

wboxtest_initcall(polyline_wbt_init);
wboxtest_exitcall(polyline_wbt_exit);
