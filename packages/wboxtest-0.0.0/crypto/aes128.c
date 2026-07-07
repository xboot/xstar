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

static void * aes128_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void aes128_clean(struct wboxtest_t * wbt, void * data)
{
}

static void aes128_run(struct wboxtest_t * wbt, void * data)
{
	struct aes128_ctx_t ctx;
	uint8_t in[AES128_BLOCK_SIZE * 10];
	uint8_t out1[AES128_BLOCK_SIZE * 10];
	uint8_t out2[AES128_BLOCK_SIZE * 10];
	uint8_t iv[AES128_BLOCK_SIZE];
	uint8_t key[AES128_KEY_SIZE];
	uint64_t offset;
	int bytes;

	wboxtest_random_buffer((char *)in, sizeof(in));
	wboxtest_random_buffer((char *)key, sizeof(key));
	aes128_set_key(&ctx, key);
	aes128_ecb_encrypt(&ctx, in, out1, 10);
	aes128_ecb_decrypt(&ctx, out1, out2, 10);
	assert_memory_equal(in, out2, sizeof(in));

	wboxtest_random_buffer((char *)in, sizeof(in));
	wboxtest_random_buffer((char *)iv, sizeof(iv));
	wboxtest_random_buffer((char *)key, sizeof(key));
	aes128_set_key(&ctx, key);
	aes128_cbc_encrypt(&ctx, iv, in, out1, 10);
	aes128_cbc_decrypt(&ctx, iv, out1, out2, 10);
	assert_memory_equal(in, out2, sizeof(in));

	offset = wboxtest_random_int(0, 10);
	bytes = wboxtest_random_int(0, sizeof(in));
	wboxtest_random_buffer((char *)in, sizeof(in));
	wboxtest_random_buffer((char *)key, sizeof(key));
	aes128_set_key(&ctx, key);
	aes128_ctr_encrypt(&ctx, offset, in, out1, bytes);
	aes128_ctr_decrypt(&ctx, offset, out1, out2, bytes);
	assert_memory_equal(in, out2, bytes);
}

static struct wboxtest_t wbt_aes128 = {
	.group	= "crypto",
	.name	= "aes128",
	.setup	= aes128_setup,
	.clean	= aes128_clean,
	.run	= aes128_run,
};

static void aes128_wbt_init(void)
{
	register_wboxtest(&wbt_aes128);
}

static void aes128_wbt_exit(void)
{
	unregister_wboxtest(&wbt_aes128);
}

wboxtest_initcall(aes128_wbt_init);
wboxtest_exitcall(aes128_wbt_exit);
