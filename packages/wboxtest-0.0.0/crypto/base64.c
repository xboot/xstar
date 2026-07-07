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

static void * base64_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void base64_clean(struct wboxtest_t * wbt, void * data)
{
}

static void base64_run(struct wboxtest_t * wbt, void * data)
{
	char in[128];
	char out1[base64_encode_size(sizeof(in))];
	char out2[base64_decode_size(sizeof(out1))];

	wboxtest_random_buffer(in, sizeof(in));
	base64_encode(in, sizeof(in), out1);
	base64_decode(out1, sizeof(out1), out2);
	assert_memory_equal(in, out2, sizeof(in));
}

static struct wboxtest_t wbt_base64 = {
	.group	= "crypto",
	.name	= "base64",
	.setup	= base64_setup,
	.clean	= base64_clean,
	.run	= base64_run,
};

static void base64_wbt_init(void)
{
	register_wboxtest(&wbt_base64);
}

static void base64_wbt_exit(void)
{
	unregister_wboxtest(&wbt_base64);
}

wboxtest_initcall(base64_wbt_init);
wboxtest_exitcall(base64_wbt_exit);
