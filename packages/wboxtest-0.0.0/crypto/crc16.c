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

static void * crc16_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void crc16_clean(struct wboxtest_t * wbt, void * data)
{
}

static void crc16_run(struct wboxtest_t * wbt, void * data)
{
	uint8_t msg[5] = { 'x', 'b', 'o', 'o', 't' };
	uint16_t crc = 0;

	crc = crc16_sum(crc, msg, sizeof(msg));
	assert_equal(crc, 0x394c);
}

static struct wboxtest_t wbt_crc16 = {
	.group	= "crypto",
	.name	= "crc16",
	.setup	= crc16_setup,
	.clean	= crc16_clean,
	.run	= crc16_run,
};

static void crc16_wbt_init(void)
{
	register_wboxtest(&wbt_crc16);
}

static void crc16_wbt_exit(void)
{
	unregister_wboxtest(&wbt_crc16);
}

wboxtest_initcall(crc16_wbt_init);
wboxtest_exitcall(crc16_wbt_exit);
