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

static void * sprintf_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void sprintf_clean(struct wboxtest_t * wbt, void * data)
{
}

static void sprintf_run(struct wboxtest_t * wbt, void * data)
{
	char buf[SZ_1K];

	xos_sprintf(buf, "%d = %i^%s", 1024, 2, "10");
	assert_string_equal(buf, "1024 = 2^10");

	xos_sprintf(buf, "0x%x = 0X%X", 0xbad, 2989);
	assert_string_equal(buf, "0xbad = 0XBAD");

	xos_sprintf(buf, "%d + %d + %d = %d", 1, 1, 778, 780);
	assert_string_equal(buf, "1 + 1 + 778 = 780");

	xos_sprintf(buf, "%d %% %u = %i", 42, 2, 0);
	assert_string_equal(buf, "42 % 2 = 0");

	xos_sprintf(buf, "%c%c%c%c%c", 'x', 'b', 'o', 'o', 't');
	assert_string_equal(buf, "xboot");

	xos_sprintf(buf, "%+d", 2989);
	assert_string_equal(buf, "+2989");

	xos_sprintf(buf, "%+d", -2989);
	assert_string_equal(buf, "-2989");

	xos_sprintf(buf, "%e", 323.567);
	assert_string_equal(buf, "3.235670e+002");

	xos_sprintf(buf, "%+e", 323.567);
	assert_string_equal(buf, "+3.235670e+002");

	xos_sprintf(buf, "%+e", -323.567);
	assert_string_equal(buf, "-3.235670e+002");
}

static struct wboxtest_t wbt_sprintf = {
	.group	= "stdio",
	.name	= "sprintf",
	.setup	= sprintf_setup,
	.clean	= sprintf_clean,
	.run	= sprintf_run,
};

static void sprintf_wbt_init(void)
{
	register_wboxtest(&wbt_sprintf);
}

static void sprintf_wbt_exit(void)
{
	unregister_wboxtest(&wbt_sprintf);
}

wboxtest_initcall(sprintf_wbt_init);
wboxtest_exitcall(sprintf_wbt_exit);
