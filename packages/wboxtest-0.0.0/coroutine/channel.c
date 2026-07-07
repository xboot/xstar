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

struct wbt_channel_pdata_t
{
	struct cochannel_t * ch;
};

static void * channel_setup(struct wboxtest_t * wbt)
{
	struct wbt_channel_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct wbt_channel_pdata_t));
	if(!pdat)
		return NULL;

	pdat->ch = cochannel_alloc(0);
	if(!pdat->ch)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	return pdat;
}

static void channel_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;

	if(pdat)
	{
		cochannel_free(pdat->ch);
		xos_mem_free(pdat);
	}
}

static void co_recv(struct scheduler_t * sched, void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;
	int cnt = 10;
	int c = 0;

	while(cnt--)
	{
		cochannel_recv(sched, pdat->ch, (unsigned char *)&c, sizeof(c));
		assert_equal(c, cnt);
	}
}

static void co_send(struct scheduler_t * sched, void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;
	int cnt = 10;

	while(cnt--)
	{
		cochannel_send(sched, pdat->ch, (unsigned char *)&cnt, sizeof(cnt));
	}
}

static void channel_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;
	struct scheduler_t sched;

	if(pdat)
	{
		scheduler_init(&sched);
		coroutine_start(&sched, co_recv, pdat, 0);
		coroutine_start(&sched, co_send, pdat, 0);

		scheduler_loop(&sched);
	}
}

static struct wboxtest_t wbt_channel = {
	.group	= "coroutine",
	.name	= "channel",
	.setup	= channel_setup,
	.clean	= channel_clean,
	.run	= channel_run,
};

static void channel_wbt_init(void)
{
	register_wboxtest(&wbt_channel);
}

static void channel_wbt_exit(void)
{
	unregister_wboxtest(&wbt_channel);
}

wboxtest_initcall(channel_wbt_init);
wboxtest_exitcall(channel_wbt_exit);
