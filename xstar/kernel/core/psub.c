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

#include <kernel/core/psub.h>

static struct psctx_t * __psub_context = NULL;

void psub_publish(const char * topic, void * pdat)
{
	ps_publish(__psub_context, topic, pdat);
}

void psub_subscribe(const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot)
{
	ps_subscribe(__psub_context, topic, cb, sdat, oneshot);
}

void psub_unsubscribe(const char * topic, void (*cb)(void *, void *), void * sdat)
{
	ps_unsubscribe(__psub_context, topic, cb, sdat);
}

static void psub_pure_init(void)
{
	__psub_context = ps_alloc(0);
}

static void psub_pure_exit(void)
{
	ps_free(__psub_context);
}

pure_initcall(psub_pure_init);
pure_exitcall(psub_pure_exit);
