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

struct wbt_semaphore_pdata_t {
	struct semaphore_t sem;
	int counter;
};

static void * semaphore_setup(struct wboxtest_t * wbt)
{
	struct wbt_semaphore_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct wbt_semaphore_pdata_t));
	if(!pdat)
		return NULL;

	xos_semaphore_init(&pdat->sem, 2);
	pdat->counter = 0;

	return pdat;
}

static void semaphore_clean(struct wboxtest_t * wbt, void * data)
{
	struct wbt_semaphore_pdata_t * pdat = (struct wbt_semaphore_pdata_t *)data;

	if(pdat)
	{
		xos_semaphore_exit(&pdat->sem);
		xos_mem_free(pdat);
	}
}

static void semaphore_thread(void * data)
{
	struct wbt_semaphore_pdata_t * pdat = (struct wbt_semaphore_pdata_t *)data;

	if(xos_semaphore_wait(&pdat->sem, 100))
	{
		pdat->counter++;
		xos_thread_sleep(50 * 1000000);
		xos_semaphore_post(&pdat->sem);
	}
}

static void semaphore_run(struct wboxtest_t * wbt, void * data)
{
	struct wbt_semaphore_pdata_t * pdat = (struct wbt_semaphore_pdata_t *)data;

	if(pdat)
	{
		struct thread_t * threads[5];
		for(int i = 0; i < 5; i++)
			threads[i] = xos_thread_create(NULL, semaphore_thread, pdat, 0);
		for(int i = 0; i < 5; i++)
		{
			xos_thread_wait(threads[i]);
			xos_thread_destroy(threads[i]);
		}
		assert_inrange(pdat->counter, 2, 5);
	}
}

static struct wboxtest_t wbt_semaphore = {
	.group	= "thread",
	.name	= "semaphore",
	.setup	= semaphore_setup,
	.clean	= semaphore_clean,
	.run	= semaphore_run,
};

static void semaphore_wbt_init(void)
{
	register_wboxtest(&wbt_semaphore);
}

static void semaphore_wbt_exit(void)
{
	unregister_wboxtest(&wbt_semaphore);
}

wboxtest_initcall(semaphore_wbt_init);
wboxtest_exitcall(semaphore_wbt_exit);
