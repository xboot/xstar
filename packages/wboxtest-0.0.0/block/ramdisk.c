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

struct wbt_ramdisk_pdata_t
{
	struct block_t * blk;
	unsigned char * rambuf;
};

static void * ramdisk_setup(struct wboxtest_t * wbt)
{
	struct wbt_ramdisk_pdata_t * pdat;
	char json[256];
	int length;

	pdat = xos_mem_malloc(sizeof(struct wbt_ramdisk_pdata_t));
	if(!pdat)
		return NULL;

	pdat->rambuf = xos_mem_malloc(SZ_1M);
	if(!pdat->rambuf)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	length = xos_sprintf(json,
		"{\"blk-ramdisk:999\":{\"address\":%lld,\"size\":%lld}}",
		(unsigned long long)((io_addr_t)pdat->rambuf),
		(unsigned long long)((io_addr_t)SZ_1M));
	probe_device(json, length);

	pdat->blk = search_block("blk-ramdisk.999");
	if(!pdat->blk)
	{
		xos_mem_free(pdat->rambuf);
		xos_mem_free(pdat);
		return NULL;
	}

	return pdat;
}

static void ramdisk_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_ramdisk_pdata_t * pdat = (struct wbt_ramdisk_pdata_t *)data;

	if(pdat)
	{
		unregister_block(pdat->blk);
		xos_mem_free(pdat->rambuf);
		xos_mem_free(pdat);
	}
}

static void ramdisk_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_ramdisk_pdata_t * pdat = (struct wbt_ramdisk_pdata_t *)data;
	uint64_t offset, length;
	char * buf1, * buf2;

	if(pdat)
	{
		offset = wboxtest_random_int(0, block_capacity(pdat->blk) - 1);
		length = wboxtest_random_int(1, SZ_1M);
		length = block_available(pdat->blk, offset, length);

		buf1 = xos_mem_malloc(length);
		buf2 = xos_mem_malloc(length);
		if(!buf1 || !buf2)
		{
			if(buf1)
				xos_mem_free(buf1);
			if(buf2)
				xos_mem_free(buf2);
		}

		wboxtest_random_buffer(buf1, length);
		block_write(pdat->blk, (uint8_t *)buf1, offset, length);
		block_sync(pdat->blk);
		block_read(pdat->blk, (uint8_t *)buf2, offset, length);
		assert_memory_equal(buf1, buf2, length);

		if(buf1)
			xos_mem_free(buf1);
		if(buf2)
			xos_mem_free(buf2);
	}
}

static struct wboxtest_t wbt_ramdisk = {
	.group	= "block",
	.name	= "ramdisk",
	.setup	= ramdisk_setup,
	.clean	= ramdisk_clean,
	.run	= ramdisk_run,
};

static void ramdisk_wbt_init(void)
{
	register_wboxtest(&wbt_ramdisk);
}

static void ramdisk_wbt_exit(void)
{
	unregister_wboxtest(&wbt_ramdisk);
}

wboxtest_initcall(ramdisk_wbt_init);
wboxtest_exitcall(ramdisk_wbt_exit);
