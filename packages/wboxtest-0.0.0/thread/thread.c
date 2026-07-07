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

static void * thread_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void thread_clean(struct wboxtest_t * wbt, void * data)
{
}

static void count_thread(void * data)
{
	int * count = (int *)data;

	xos_thread_sleep(100 * 1000000);
	*count = 1234;
}

static void thread_run(struct wboxtest_t * wbt, void * data)
{
	int count = 0;
	struct thread_t * thread = xos_thread_create(NULL, count_thread, &count, 0);

	xos_thread_wait(thread);
	xos_thread_destroy(thread);
	assert_equal(count, 1234);
}

static struct wboxtest_t wbt_thread = {
	.group	= "thread",
	.name	= "thread",
	.setup	= thread_setup,
	.clean	= thread_clean,
	.run	= thread_run,
};

static void thread_wbt_init(void)
{
	register_wboxtest(&wbt_thread);
}

static void thread_wbt_exit(void)
{
	unregister_wboxtest(&wbt_thread);
}

wboxtest_initcall(thread_wbt_init);
wboxtest_exitcall(thread_wbt_exit);
