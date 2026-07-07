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

static void * basename_setup(struct wboxtest_t * wbt)
{
	return NULL;
}

static void basename_clean(struct wboxtest_t * wbt, void * data)
{
}

static void basename_run(struct wboxtest_t * wbt, void * data)
{
	char path[CONFIG_XSTAR_MAX_PATH];

	xos_sprintf(path, "%s", "/usr/lib");
	assert_string_equal(path_basename(path), "lib");

	xos_sprintf(path, "%s", "/usr/");
	assert_string_equal(path_basename(path), "usr");

	xos_sprintf(path, "%s", "usr");
	assert_string_equal(path_basename(path), "usr");

	xos_sprintf(path, "%s", "/");
	assert_string_equal(path_basename(path), "/");

	xos_sprintf(path, "%s", ".");
	assert_string_equal(path_basename(path), ".");

	xos_sprintf(path, "%s", "..");
	assert_string_equal(path_basename(path), "..");
}

static struct wboxtest_t wbt_basename = {
	.group	= "path",
	.name	= "basename",
	.setup	= basename_setup,
	.clean	= basename_clean,
	.run	= basename_run,
};

static void basename_wbt_init(void)
{
	register_wboxtest(&wbt_basename);
}

static void basename_wbt_exit(void)
{
	unregister_wboxtest(&wbt_basename);
}

wboxtest_initcall(basename_wbt_init);
wboxtest_exitcall(basename_wbt_exit);
