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

static void * sha256_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void sha256_clean(struct wboxtest_t * wbt, void * data)
{
}

static void sha256_run(struct wboxtest_t * wbt, void * data)
{
	uint8_t msg[5] = { 'x', 'b', 'o', 'o', 't' };
	uint8_t digest[SHA256_DIGEST_SIZE] = {
		0xa8, 0x1a, 0x87, 0xbd, 0xa6, 0x8a, 0xbf, 0x88,
		0x8b, 0x64, 0xaa, 0x20, 0xc6, 0xcf, 0x20, 0x4b,
		0x91, 0x88, 0x9a, 0xfa, 0x1a, 0x85, 0x06, 0xb1,
		0xe4, 0x12, 0x5a, 0xcb, 0x37, 0x2e, 0xdb, 0x0d,
	};
	uint8_t result[SHA256_DIGEST_SIZE];

	sha256_hash(msg, sizeof(msg), result);
	assert_memory_equal(result, digest, SHA256_DIGEST_SIZE);
}

static struct wboxtest_t wbt_sha256 = {
	.group	= "crypto",
	.name	= "sha256",
	.setup	= sha256_setup,
	.clean	= sha256_clean,
	.run	= sha256_run,
};

static void sha256_wbt_init(void)
{
	register_wboxtest(&wbt_sha256);
}

static void sha256_wbt_exit(void)
{
	unregister_wboxtest(&wbt_sha256);
}

wboxtest_initcall(sha256_wbt_init);
wboxtest_exitcall(sha256_wbt_exit);
