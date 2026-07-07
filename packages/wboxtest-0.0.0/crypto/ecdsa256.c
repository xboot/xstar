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

static void * ecdsa256_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void ecdsa256_clean(struct wboxtest_t * wbt, void * data)
{
}

static void ecdsa256_run(struct wboxtest_t * wbt, void * data)
{
	uint8_t pub[ECDSA256_PUBLIC_KEY_SIZE];
	uint8_t priv[ECDSA256_PRIVATE_KEY_SIZE];
	uint8_t sign[ECDSA256_SIGNATURE_SIZE];
	uint8_t msg[32];

	wboxtest_random_buffer((char *)msg, sizeof(msg));
	assert_true(ecdsa256_keygen(pub, priv));
	assert_true(ecdsa256_sign(priv, msg, sign));
	assert_true(ecdsa256_verify(pub, msg, sign));
}

static struct wboxtest_t wbt_ecdsa256 = {
	.group	= "crypto",
	.name	= "ecdsa256",
	.setup	= ecdsa256_setup,
	.clean	= ecdsa256_clean,
	.run	= ecdsa256_run,
};

static void ecdsa256_wbt_init(void)
{
	register_wboxtest(&wbt_ecdsa256);
}

static void ecdsa256_wbt_exit(void)
{
	unregister_wboxtest(&wbt_ecdsa256);
}

wboxtest_initcall(ecdsa256_wbt_init);
wboxtest_exitcall(ecdsa256_wbt_exit);
