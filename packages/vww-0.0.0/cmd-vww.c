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

#include <xstar.h>
#include <vww.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    vww <image>\r\n");
}

static int do_vww(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 1, 1))
	{
		usage();
		return -1;
	}

	const char * image = sarg_at(&sarg, 0);
	char fpath[CONFIG_XSTAR_MAX_PATH];
	if((shell_realpath(image, fpath) < 0) || !xfs_isfile(shell_getxfs(), fpath))
	{
		shell_printf("cannot access %s: No such file\r\n", image);
		return -1;
	}

	struct surface_t * s = surface_alloc_from_xfs(shell_getxfs(), fpath);
	if(s)
	{
		struct vision_t * v = vision_alloc(VISION_TYPE_RGB, surface_get_width(s), surface_get_height(s));
		if(v)
		{
			vision_apply_surface(v, s);
			shell_printf("person: %d%%\r\n", vision_vww(v));
			vision_free(v);
		}
		surface_free(s);
	}
	return 0;
}

static struct command_t cmd_vww = {
	.name	= "vww",
	.desc	= "visual wake words detection",
	.usage	= usage,
	.exec	= do_vww,
};

static void vww_cmd_init(void)
{
	register_command(&cmd_vww);
}

static void vww_cmd_exit(void)
{
	unregister_command(&cmd_vww);
}

command_initcall(vww_cmd_init);
command_exitcall(vww_cmd_exit);
