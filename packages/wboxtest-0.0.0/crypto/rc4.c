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

static void * rc4_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void rc4_clean(struct wboxtest_t * wbt, void * data)
{
}

static void rc4_run(struct wboxtest_t * wbt, void * data)
{
	uint8_t key[5] = { 'x', 'b', 'o', 'o', 't' };
	uint8_t dat[256];
	uint8_t tmp[256];

	wboxtest_random_buffer((char *)dat, sizeof(dat));
	xos_memcpy(tmp, dat, sizeof(dat));
	rc4_crypt(key, 5, tmp, sizeof(tmp));
	rc4_crypt(key, 5, tmp, sizeof(tmp));
	assert_memory_equal(dat, tmp, sizeof(dat));
}

static struct wboxtest_t wbt_rc4 = {
	.group	= "crypto",
	.name	= "rc4",
	.setup	= rc4_setup,
	.clean	= rc4_clean,
	.run	= rc4_run,
};

static void rc4_wbt_init(void)
{
	register_wboxtest(&wbt_rc4);
}

static void rc4_wbt_exit(void)
{
	unregister_wboxtest(&wbt_rc4);
}

wboxtest_initcall(rc4_wbt_init);
wboxtest_exitcall(rc4_wbt_exit);
