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

#include <driver/clocksource/clocksource.h>
#include <kernel/time/delay.h>
#include <kernel/window/window.h>
#include <kernel/shell/context.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    iplay [dir|file] [-t=millisecond] [-m=none|contain|cover|fill] [-c=color] [-d=framebuffer]\r\n");
}

enum iplay_mode_t {
	IPLAY_MODE_NONE		= 0,
	IPLAY_MODE_CONTAIN	= 1,
	IPLAY_MODE_COVER	= 2,
	IPLAY_MODE_FILL		= 3,
};

static void cb(const char * dir, const char * name, void * data)
{
	struct slist_t * sl = (struct slist_t *)data;
	if(name && (name[0] != '.'))
		slist_add(sl, NULL, "%s/%s", dir, name);
}

static void display(struct window_t * w, const char * path, int ms, enum iplay_mode_t mode, struct color_t * c)
{
	struct slist_t * sl = slist_alloc();
	if(sl)
	{
		struct slist_t * e;
		if(xfs_isdir(shell_getxfs(), path))
		{
			xfs_walk(shell_getxfs(), path, cb, sl);
		}
		else if(xfs_isfile(shell_getxfs(), path))
		{
			slist_add(sl, NULL, "%s", path);
		}
		else
		{
			shell_printf("cannot access %s: No such file or directory\r\n", path);
			slist_free(sl);
			return;
		}
		slist_natsort(sl);
		slist_for_each_entry(e, sl)
		{
			ktime_t timeout = ktime_add_ms(ktime_get(), ms);
			if(xfs_isfile(shell_getxfs(), e->key))
			{
				struct surface_t * s = surface_alloc_from_xfs(shell_getxfs(), e->key);
				if(s)
				{
					window_dirtylist_fullscreen(w);
					window_present_clear(w);
					{
						struct matrix2d_t m;
						float sx, sy;
						switch(mode)
						{
						case IPLAY_MODE_NONE:
							sx = 1.0;
							sy = 1.0;
							break;
						case IPLAY_MODE_CONTAIN:
							sx = (float)window_get_width(w) / (float)surface_get_width(s);
							sy = (float)window_get_height(w) / (float)surface_get_height(s);
							if(sx >= sy)
								sx = sy;
							else
								sy = sx;
							break;
						case IPLAY_MODE_COVER:
							sx = (float)window_get_width(w) / (float)surface_get_width(s);
							sy = (float)window_get_height(w) / (float)surface_get_height(s);
							if(sx <= sy)
								sx = sy;
							else
								sy = sx;
							break;
						case IPLAY_MODE_FILL:
							sx = (float)window_get_width(w) / (float)surface_get_width(s);
							sy = (float)window_get_height(w) / (float)surface_get_height(s);
							break;
						default:
							sx = 1.0;
							sy = 1.0;
							break;
						}
						matrix2d_init_translate(&m, 0 + window_get_width(w) / 2, 0 + window_get_height(w) / 2);
						matrix2d_translate(&m, -surface_get_width(s) / 2 * sx, -surface_get_height(s) / 2 * sy);
						matrix2d_scale(&m, sx, sy);
						surface_fill(window_get_surface(w), NULL, &(struct matrix2d_t){1, 0, 0, 1, 0, 0}, window_get_width(w), window_get_height(w), c);
						surface_blit(window_get_surface(w), NULL, &m, s);
					}
					window_present_commit(w);
					surface_free(s);
					while(ktime_before(ktime_get(), timeout));
				}
			}
		}
		slist_free(sl);
	}
}

static int do_iplay(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char *[]){ "-t", "-m", "-c", "-d", NULL }, 0, 1))
	{
		usage();
		return -1;
	}

	struct window_t * w = NULL;
	const char * file = sarg_at(&sarg, 0);
	if(!file)
		file = ".";
	const char * fb = sarg_get(&sarg, "-d", NULL);
	int ms = sarg_get_uint(&sarg, "-t", 0);
	enum iplay_mode_t mode = IPLAY_MODE_NONE;
	const char * m = sarg_get(&sarg, "-m", NULL);
	if(m)
	{
		switch(shash(m))
		{
		case 0x7c9b47f5: /* "none" */
			mode = IPLAY_MODE_NONE;
			break;
		case 0xd37987d1: /* "contain" */
			mode = IPLAY_MODE_CONTAIN;
			break;
		case 0x0f3d5b84: /* "cover" */
			mode = IPLAY_MODE_COVER;
			break;
		case 0x7c96cb2c: /* "fill" */
			mode = IPLAY_MODE_FILL;
			break;
		default:
			break;
		}
	}
	struct color_t c = { 0x00, 0x00, 0x00, 0x00 };
	const char * col = sarg_get(&sarg, "-c", NULL);
	if(col)
		color_init_string(&c, col);
	if((w = window_alloc(fb, "", -1)))
	{
		char fpath[CONFIG_XSTAR_MAX_PATH];
		if(shell_realpath(file, fpath) >= 0)
			display(w, fpath, ms, mode, &c);
		window_free(w);
	}
	return 0;
}

static struct command_t cmd_iplay = {
	.name	= "iplay",
	.desc	= "display images on the screen",
	.usage	= usage,
	.exec	= do_iplay,
};

static void iplay_cmd_init(void)
{
	register_command(&cmd_iplay);
}

static void iplay_cmd_exit(void)
{
	unregister_command(&cmd_iplay);
}

command_initcall(iplay_cmd_init);
command_exitcall(iplay_cmd_exit);
