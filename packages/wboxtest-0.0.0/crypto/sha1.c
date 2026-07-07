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

static void * sha1_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void sha1_clean(struct wboxtest_t * wbt, void * data)
{
}

static void sha1_run(struct wboxtest_t * wbt, void * data)
{
	uint8_t msg[5] = { 'x', 'b', 'o', 'o', 't' };
	uint8_t digest[SHA1_DIGEST_SIZE] = {
		0x71, 0x82, 0xdf, 0x07, 0xc7, 0xf2, 0x06, 0x75,
		0x0f, 0xe2, 0xed, 0x98, 0x58, 0x0a, 0xb5, 0x26,
		0xcb, 0x83, 0x9f, 0x5d,
	};
	uint8_t result[SHA1_DIGEST_SIZE];

	sha1_hash(msg, sizeof(msg), result);
	assert_memory_equal(result, digest, SHA1_DIGEST_SIZE);
}

static struct wboxtest_t wbt_sha1 = {
	.group	= "crypto",
	.name	= "sha1",
	.setup	= sha1_setup,
	.clean	= sha1_clean,
	.run	= sha1_run,
};

static void sha1_wbt_init(void)
{
	register_wboxtest(&wbt_sha1);
}

static void sha1_wbt_exit(void)
{
	unregister_wboxtest(&wbt_sha1);
}

wboxtest_initcall(sha1_wbt_init);
wboxtest_exitcall(sha1_wbt_exit);
