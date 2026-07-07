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

struct wbt_memmove_pdata_t
{
	char * src;
	char * dst;
	size_t size;

	ktime_t t1;
	ktime_t t2;
	int calls;
};

static void * memmove_setup(struct wboxtest_t * wbt)
{
	struct wbt_memmove_pdata_t * pdat;
	int i;

	pdat = xos_mem_malloc(sizeof(struct wbt_memmove_pdata_t));
	if(!pdat)
		return NULL;

	pdat->size = SZ_1M;
	pdat->src = xos_mem_malloc(pdat->size);
	pdat->dst = xos_mem_malloc(pdat->size);
	if(!pdat->src || !pdat->dst)
	{
		xos_mem_free(pdat->src);
		xos_mem_free(pdat->dst);
		xos_mem_free(pdat);
		return NULL;
	}
	for(i = 0; i < pdat->size; i++)
	{
		pdat->src[i] = i & 0xff;
		pdat->dst[i] = 0;
	}

	return pdat;
}

static void memmove_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_memmove_pdata_t * pdat = (struct wbt_memmove_pdata_t *)data;

	if(pdat)
	{
		xos_mem_free(pdat->dst);
		xos_mem_free(pdat->src);
		xos_mem_free(pdat);
	}
}

static void memmove_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_memmove_pdata_t * pdat = (struct wbt_memmove_pdata_t *)data;
	char buf[32];

	if(pdat)
	{
		pdat->calls = 0;
		pdat->t2 = pdat->t1 = ktime_get();
		do {
			pdat->calls++;
			xos_memmove(pdat->dst, pdat->src, pdat->size);
			pdat->t2 = ktime_get();
		} while(ktime_before(pdat->t2, ktime_add_ms(pdat->t1, 2000)));
		wboxtest_printf(" Bandwidth: %s/s\r\n", xos_ssize(buf, (double)(pdat->calls * pdat->size) * 1000.0 / ktime_ms_delta(pdat->t2, pdat->t1)));
	}
}

static struct wboxtest_t wbt_memmove = {
	.group	= "benchmark-memory",
	.name	= "memmove",
	.setup	= memmove_setup,
	.clean	= memmove_clean,
	.run	= memmove_run,
};

static void memmove_wbt_init(void)
{
	register_wboxtest(&wbt_memmove);
}

static void memmove_wbt_exit(void)
{
	unregister_wboxtest(&wbt_memmove);
}

wboxtest_initcall(memmove_wbt_init);
wboxtest_exitcall(memmove_wbt_exit);
