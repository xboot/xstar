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

static void * sleep_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void sleep_clean(struct wboxtest_t * wbt, void * data)
{
}

static void co_sleep(struct scheduler_t * sched, void * data)
{
	int * pdat = (int*)data;
	int count = 10;

	while(count--)
	{
		wboxtest_printf("coroutine [%d ms]: count = %d\r\n", pdat[0], count);
		coroutine_msleep(sched, pdat[0]);
	}
}

static void sleep_run(struct wboxtest_t * wbt, void * data)
{
	struct scheduler_t sched;

	scheduler_init(&sched);
	coroutine_start(&sched, co_sleep, &(int[]){ 100 }, 0);
	coroutine_start(&sched, co_sleep, &(int[]){ 200 }, 0);
	coroutine_start(&sched, co_sleep, &(int[]){ 300 }, 0);

	scheduler_loop(&sched);
}

static struct wboxtest_t wbt_sleep = {
	.group	= "coroutine",
	.name	= "sleep",
	.setup	= sleep_setup,
	.clean	= sleep_clean,
	.run	= sleep_run,
};

static void sleep_wbt_init(void)
{
	register_wboxtest(&wbt_sleep);
}

static void sleep_wbt_exit(void)
{
	unregister_wboxtest(&wbt_sleep);
}

wboxtest_initcall(sleep_wbt_init);
wboxtest_exitcall(sleep_wbt_exit);
