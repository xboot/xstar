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

struct wbt_channel_pdata_t {
	struct thchannel_t * ch;
};

static void * channel_setup(struct wboxtest_t * wbt)
{
	struct wbt_channel_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct wbt_channel_pdata_t));
	if(!pdat)
		return NULL;

	pdat->ch = thchannel_alloc(0);
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
		thchannel_free(pdat->ch);
		xos_mem_free(pdat);
	}
}

static void th_recv(void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;
	int cnt = 10;
	int c = 0;

	while(cnt--)
	{
		thchannel_recv(pdat->ch, (unsigned char *)&c, sizeof(c), -1);
		assert_equal(c, cnt);
	}
}

static void th_send(void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;
	int cnt = 10;

	while(cnt--)
	{
		thchannel_send(pdat->ch, (unsigned char *)&cnt, sizeof(cnt), -1);
	}
}

static void th_recv_big(void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;
	unsigned char buf[64];
	int i, j;

	for(i = 0; i < 8; i++)
	{
		assert_equal(thchannel_recv(pdat->ch, buf, sizeof(buf), -1), sizeof(buf));
		for(j = 0; j < (int)sizeof(buf); j++)
			assert_equal(buf[j], (unsigned char)(i + j));
	}
}

static void th_send_big(void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;
	unsigned char buf[64];
	int i, j;

	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < (int)sizeof(buf); j++)
			buf[j] = (unsigned char)(i + j);
		assert_equal(thchannel_send(pdat->ch, buf, sizeof(buf), -1), sizeof(buf));
	}
}

static void channel_run_threads(struct wbt_channel_pdata_t * pdat, void (*fn_recv)(void *), void (*fn_send)(void *))
{
	struct thread_t * rthread = xos_thread_create(NULL, fn_recv, pdat, 0);
	struct thread_t * sthread = xos_thread_create(NULL, fn_send, pdat, 0);
	xos_thread_wait(rthread);
	xos_thread_destroy(rthread);
	xos_thread_wait(sthread);
	xos_thread_destroy(sthread);
}

static void channel_run_state(struct wbt_channel_pdata_t * pdat)
{
	unsigned char buf[4] = { 0, 1, 2, 3 };

	assert_true(thchannel_isempty(pdat->ch));
	assert_equal(thchannel_length(pdat->ch), 0);
	assert_equal(thchannel_available(pdat->ch), 16);
	assert_equal(thchannel_recv(pdat->ch, buf, sizeof(buf), 10), 0);

	assert_equal(thchannel_send(pdat->ch, buf, sizeof(buf), 10), sizeof(buf));
	assert_false(thchannel_isempty(pdat->ch));
	assert_equal(thchannel_length(pdat->ch), 4);
	assert_equal(thchannel_send(pdat->ch, buf, sizeof(buf), 10), sizeof(buf));
	assert_equal(thchannel_send(pdat->ch, buf, sizeof(buf), 10), sizeof(buf));
	assert_equal(thchannel_send(pdat->ch, buf, sizeof(buf), 10), sizeof(buf));
	assert_true(thchannel_isfull(pdat->ch));
	assert_equal(thchannel_available(pdat->ch), 0);
	assert_equal(thchannel_size(pdat->ch), 16);
	assert_equal(thchannel_send(pdat->ch, buf, sizeof(buf), 10), 0);

	thchannel_reset(pdat->ch);
	assert_true(thchannel_isempty(pdat->ch));
	assert_equal(thchannel_length(pdat->ch), 0);
	assert_equal(thchannel_available(pdat->ch), 16);
	assert_equal(thchannel_recv(pdat->ch, buf, sizeof(buf), 10), 0);
}

static void channel_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_channel_pdata_t * pdat = (struct wbt_channel_pdata_t *)data;

	if(pdat)
	{
		channel_run_threads(pdat, th_recv, th_send);
		thchannel_reset(pdat->ch);
		channel_run_threads(pdat, th_recv_big, th_send_big);
		thchannel_reset(pdat->ch);
		channel_run_state(pdat);
	}
}

static struct wboxtest_t wbt_channel = {
	.group	= "thread",
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
