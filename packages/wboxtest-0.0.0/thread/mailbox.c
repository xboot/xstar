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

struct wbt_mailbox_pdata_t {
	struct thmailbox_t * mb;
};

static void * mailbox_setup(struct wboxtest_t * wbt)
{
	struct wbt_mailbox_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct wbt_mailbox_pdata_t));
	if(!pdat)
		return NULL;

	pdat->mb = thmailbox_alloc(64, 8);
	if(!pdat->mb)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	return pdat;
}

static void mailbox_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_mailbox_pdata_t * pdat = (struct wbt_mailbox_pdata_t *)data;

	if(pdat)
	{
		thmailbox_free(pdat->mb);
		xos_mem_free(pdat);
	}
}

static void th_recv(void * data)
{
	struct wbt_mailbox_pdata_t * pdat = (struct wbt_mailbox_pdata_t *)data;
	int cnt = 10;

	while(cnt--)
	{
		int v = 0;
		void * p = NULL;

		assert_equal(thmailbox_recv(pdat->mb, &p, -1), sizeof(v));
		v = *(int *)p;
		thmailbox_buf_free(pdat->mb, p);
		assert_equal(v, cnt);
	}
}

static void th_send(void * data)
{
	struct wbt_mailbox_pdata_t * pdat = (struct wbt_mailbox_pdata_t *)data;
	int cnt = 10;

	while(cnt--)
	{
		int * v = (int *)thmailbox_buf_alloc(pdat->mb, sizeof(int), -1);

		assert_not_null(v);
		*v = cnt;
		assert_equal(thmailbox_send(pdat->mb, v, sizeof(int), -1), sizeof(int));
	}
}

static void th_recv_big(void * data)
{
	struct wbt_mailbox_pdata_t * pdat = (struct wbt_mailbox_pdata_t *)data;
	unsigned char expect[64];
	int i, j;

	for(i = 0; i < 8; i++)
	{
		void * p = NULL;

		assert_equal(thmailbox_recv(pdat->mb, &p, -1), sizeof(expect));
		for(j = 0; j < (int)sizeof(expect); j++)
			expect[j] = (unsigned char)(i + j);
		assert_memory_equal(p, expect, sizeof(expect));
		thmailbox_buf_free(pdat->mb, p);
	}
}

static void th_send_big(void * data)
{
	struct wbt_mailbox_pdata_t * pdat = (struct wbt_mailbox_pdata_t *)data;
	int i, j;

	for(i = 0; i < 8; i++)
	{
		unsigned char * buf = (unsigned char *)thmailbox_buf_alloc(pdat->mb, 64, -1);

		assert_not_null(buf);
		for(j = 0; j < 64; j++)
			buf[j] = (unsigned char)(i + j);
		assert_equal(thmailbox_send(pdat->mb, buf, 64, -1), 64);
	}
}

static void mailbox_run_threads(struct wbt_mailbox_pdata_t * pdat, void (*fn_recv)(void *), void (*fn_send)(void *))
{
	struct thread_t * rthread = xos_thread_create(NULL, fn_recv, pdat, 0);
	struct thread_t * sthread = xos_thread_create(NULL, fn_send, pdat, 0);
	xos_thread_wait(rthread);
	xos_thread_destroy(rthread);
	xos_thread_wait(sthread);
	xos_thread_destroy(sthread);
}

static void mailbox_run_state(struct wbt_mailbox_pdata_t * pdat)
{
	void * b[8];
	void * p;
	int i;

	assert_equal(thmailbox_buf_available(pdat->mb), 8);
	assert_equal(thmailbox_recv(pdat->mb, &p, 10), 0);
	assert_equal(thmailbox_send(pdat->mb, NULL, 64, 10), 0);

	assert_null(thmailbox_buf_alloc(pdat->mb, 65, 0));
	assert_equal(thmailbox_buf_available(pdat->mb), 8);
	for(i = 0; i < 8; i++)
	{
		b[i] = thmailbox_buf_alloc(pdat->mb, 64, 0);
		assert_not_null(b[i]);
		assert_equal(thmailbox_send(pdat->mb, b[i], 64, 10), 64);
	}
	assert_equal(thmailbox_buf_available(pdat->mb), 0);
	assert_null(thmailbox_buf_alloc(pdat->mb, 64, 0));
	assert_null(thmailbox_buf_alloc(pdat->mb, 64, 10));

	for(i = 0; i < 8; i++)
	{
		assert_equal(thmailbox_recv(pdat->mb, &p, 10), 64);
		assert_equal(p, b[i]);
		thmailbox_buf_free(pdat->mb, p);
		assert_equal(thmailbox_buf_available(pdat->mb), i + 1);
	}
	assert_equal(thmailbox_recv(pdat->mb, &p, 10), 0);

	thmailbox_reset(pdat->mb);
	assert_equal(thmailbox_buf_available(pdat->mb), 8);
	assert_equal(thmailbox_recv(pdat->mb, &p, 10), 0);

	for(i = 0; i < 8; i++)
	{
		b[i] = thmailbox_buf_alloc(pdat->mb, 64, 0);
		assert_not_null(b[i]);
	}
	for(i = 0; i < 8; i++)
		thmailbox_buf_free(pdat->mb, b[i]);
}

static void mailbox_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_mailbox_pdata_t * pdat = (struct wbt_mailbox_pdata_t *)data;

	if(pdat)
	{
		mailbox_run_threads(pdat, th_recv, th_send);
		mailbox_run_threads(pdat, th_recv_big, th_send_big);
		mailbox_run_state(pdat);
	}
}

static struct wboxtest_t wbt_mailbox = {
	.group	= "thread",
	.name	= "mailbox",
	.setup	= mailbox_setup,
	.clean	= mailbox_clean,
	.run	= mailbox_run,
};

static void mailbox_wbt_init(void)
{
	register_wboxtest(&wbt_mailbox);
}

static void mailbox_wbt_exit(void)
{
	unregister_wboxtest(&wbt_mailbox);
}

wboxtest_initcall(mailbox_wbt_init);
wboxtest_exitcall(mailbox_wbt_exit);
