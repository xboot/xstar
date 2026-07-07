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

struct wbt_benchmark_pdata_t
{
	char * src;
	char * dst;
	size_t size;

	ktime_t t1;
	ktime_t t2;
	int calls;
	int dma;
};

static void * benchmark_setup(struct wboxtest_t * wbt)
{
	struct wbt_benchmark_pdata_t * pdat;
	int dma = 0;
	int i;

	if(!dma_is_valid(dma))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct wbt_benchmark_pdata_t));
	if(!pdat)
		return NULL;

	pdat->size = SZ_256K;
	pdat->src = xos_dma_alloc_coherent(pdat->size);
	pdat->dst = xos_dma_alloc_coherent(pdat->size);
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
	pdat->dma = dma;
	dma_stop(pdat->dma);

	return pdat;
}

static void benchmark_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_benchmark_pdata_t * pdat = (struct wbt_benchmark_pdata_t *)data;

	if(pdat)
	{
		xos_dma_free_coherent(pdat->dst);
		xos_dma_free_coherent(pdat->src);
		xos_mem_free(pdat);
	}
}

static void benchmark_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_benchmark_pdata_t * pdat = (struct wbt_benchmark_pdata_t *)data;
	char buf[32];
	int flag;
	if(pdat)
	{
		flag = DMA_S_TYPE(DMA_TYPE_MEMTOMEM);
		flag |= DMA_S_SRC_INC(DMA_INCREASE) | DMA_S_DST_INC(DMA_INCREASE);
		flag |= DMA_S_SRC_WIDTH(DMA_WIDTH_8BIT) | DMA_S_DST_WIDTH(DMA_WIDTH_8BIT);
		flag |= DMA_S_SRC_BURST(DMA_BURST_SIZE_1) | DMA_S_DST_BURST(DMA_BURST_SIZE_1);
		flag |= DMA_S_SRC_PORT(0) | DMA_S_DST_PORT(0);
		pdat->calls = 0;
		pdat->t2 = pdat->t1 = ktime_get();
		do {
			pdat->calls++;
			dma_start(pdat->dma, pdat->src, pdat->dst, pdat->size, flag, NULL, NULL, NULL);
			dma_wait(pdat->dma);
			pdat->t2 = ktime_get();
		} while(ktime_before(pdat->t2, ktime_add_ms(pdat->t1, 2000)));
		wboxtest_printf(" Bandwidth: %s/s\r\n", xos_ssize(buf, (double)(pdat->calls * pdat->size) * 1000.0 / ktime_ms_delta(pdat->t2, pdat->t1)));
	}
}

static struct wboxtest_t wbt_benchmark = {
	.group	= "dma",
	.name	= "benchmark",
	.setup	= benchmark_setup,
	.clean	= benchmark_clean,
	.run	= benchmark_run,
};

static void benchmark_wbt_init(void)
{
	register_wboxtest(&wbt_benchmark);
}

static void benchmark_wbt_exit(void)
{
	unregister_wboxtest(&wbt_benchmark);
}

wboxtest_initcall(benchmark_wbt_init);
wboxtest_exitcall(benchmark_wbt_exit);
