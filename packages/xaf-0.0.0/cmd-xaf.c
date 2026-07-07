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

#include <xaf.h>
#include <kernel/command/command.h>

enum xaf_display_mode_t {
	XAF_DISPLAY_MODE_NONE		= 0,
	XAF_DISPLAY_MODE_CONTAIN	= 1,
	XAF_DISPLAY_MODE_COVER		= 2,
	XAF_DISPLAY_MODE_FILL		= 3,
};

struct xaf_context_t {
	struct window_t * window;
	struct surface_t * surface;
	struct matrix2d_t matrix;
	struct xaf_t * xaf;
	uint8_t * data;
	uint32_t size;
	int quit;
};

static struct xaf_context_t * xaf_context_alloc(const char * fb, const char * input)
{
	struct xaf_context_t * ctx;

	ctx = xos_mem_malloc(sizeof(struct xaf_context_t));
	if(!ctx)
		return ctx;

	ctx->window = window_alloc(fb, input, -1);
	ctx->surface = NULL;
	ctx->xaf = NULL;
	ctx->data = NULL;
	ctx->size = 0;
	ctx->quit = 0;
	return ctx;
}

static void xaf_context_free(struct xaf_context_t * ctx)
{
	if(ctx)
	{
		if(ctx->xaf)
			xaf_free(ctx->xaf);
		if(ctx->data)
			xos_mem_free(ctx->data);
		if(ctx->surface)
			surface_free(ctx->surface);
		if(ctx->window)
			window_free(ctx->window);
		xos_mem_free(ctx);
	}
}

static void xaf_set_display_mode(struct xaf_context_t * ctx, enum xaf_display_mode_t mode)
{
	if(ctx && ctx->window && ctx->surface)
	{
		float sx = 1.0, sy = 1.0;
		switch(mode)
		{
		case XAF_DISPLAY_MODE_NONE:
			sx = 1.0;
			sy = 1.0;
			break;
		case XAF_DISPLAY_MODE_CONTAIN:
			sx = (float)window_get_width(ctx->window) / (float)surface_get_width(ctx->surface);
			sy = (float)window_get_height(ctx->window) / (float)surface_get_height(ctx->surface);
			if(sx >= sy)
				sx = sy;
			else
				sy = sx;
			break;
		case XAF_DISPLAY_MODE_COVER:
			sx = (float)window_get_width(ctx->window) / (float)surface_get_width(ctx->surface);
			sy = (float)window_get_height(ctx->window) / (float)surface_get_height(ctx->surface);
			if(sx <= sy)
				sx = sy;
			else
				sy = sx;
			break;
		case XAF_DISPLAY_MODE_FILL:
			sx = (float)window_get_width(ctx->window) / (float)surface_get_width(ctx->surface);
			sy = (float)window_get_height(ctx->window) / (float)surface_get_height(ctx->surface);
			break;
		default:
			break;
		}
		matrix2d_init_translate(&ctx->matrix, 0 + window_get_width(ctx->window) / 2, 0 + window_get_height(ctx->window) / 2);
		matrix2d_translate(&ctx->matrix, -surface_get_width(ctx->surface) / 2 * sx, -surface_get_height(ctx->surface) / 2 * sy);
		matrix2d_scale(&ctx->matrix, sx, sy);
	}
}

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    xaf <file> [-m=none|contain|cover|fill] [-c=color] [-f=framebuffer] [-i=input] [-lock] [-hide] [-loop]\r\n");
}

static int do_xaf(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char *[]){ "-m", "-c", "-f", "-i", "-lock", "-hide", "-loop", NULL }, 0, 1))
	{
		usage();
		return -1;
	}

	char fpath[CONFIG_XSTAR_MAX_PATH];
	const char * filename = sarg_at(&sarg, 0);
	const char * fb = sarg_get(&sarg, "-f", NULL);
	const char * input = sarg_get(&sarg, "-i", NULL);
	int lock = sarg_has(&sarg, "-lock");
	int hide = sarg_has(&sarg, "-hide");
	int loop = sarg_has(&sarg, "-loop");
	enum xaf_display_mode_t mode = XAF_DISPLAY_MODE_NONE;
	const char * m = sarg_get(&sarg, "-m", NULL);
	if(m)
	{
		switch(shash(m))
		{
		case 0x7c9b47f5: /* "none" */
			mode = XAF_DISPLAY_MODE_NONE;
			break;
		case 0xd37987d1: /* "contain" */
			mode = XAF_DISPLAY_MODE_CONTAIN;
			break;
		case 0x0f3d5b84: /* "cover" */
			mode = XAF_DISPLAY_MODE_COVER;
			break;
		case 0x7c96cb2c: /* "fill" */
			mode = XAF_DISPLAY_MODE_FILL;
			break;
		default:
			break;
		}
	}
	struct color_t color = { 0x00, 0x00, 0x00, 0xff };
	const char * col = sarg_get(&sarg, "-c", NULL);
	if(col)
		color_init_string(&color, col);
	if(!filename)
	{
		usage();
		return -1;
	}
	if(shell_realpath(filename, fpath) < 0)
	{
		usage();
		return -1;
	}
	struct xaf_context_t * ctx = xaf_context_alloc(fb, input);
	if(ctx)
	{
		struct xfs_file_t * file = xfs_open_read(shell_getxfs(), fpath);
		if(file)
		{
			ctx->size = xfs_length(file);
			ctx->data = xos_mem_malloc(ctx->size);
			if(ctx->data)
			{
				xfs_read(file, ctx->data, ctx->size);
				ctx->xaf = xaf_alloc(ctx->data, ctx->size);
			}
			xfs_close(file);
		}

		if(ctx->xaf)
		{
			ctx->surface = surface_alloc(xaf_get_width(ctx->xaf), xaf_get_height(ctx->xaf));
			xaf_set_display_mode(ctx, mode);

			uint32_t width = xaf_get_width(ctx->xaf);
			uint32_t height = xaf_get_height(ctx->xaf);
			uint8_t fps = xaf_get_fps(ctx->xaf);
			double frame_duration = (fps > 0) ? (1.0 / (double)fps) : (1.0 / 30.0);
			double timestamp = (double)ktime_to_ns(ktime_get()) / 1000000000.0;

			while(!ctx->quit)
			{
				struct event_t e;
				if(window_pump_event(ctx->window, &e) && !lock)
				{
					switch(e.type)
					{
					case EVENT_TYPE_KEY_DOWN:
						switch(e.e.key_down.key)
						{
						case KB_KEY_HOME:
							ctx->quit = 1;
							break;
						case KB_KEY_SPACE:
							mode = (mode + 1) & 0x3;
							xaf_set_display_mode(ctx, mode);
							break;
						default:
							break;
						}
						break;
					case EVENT_TYPE_KEY_UP:
						break;
					case EVENT_TYPE_SYSTEM_EXIT:
						ctx->quit = 1;
						break;
					default:
						break;
					}
				}

				double now = (double)ktime_to_ns(ktime_get()) / 1000000000.0;
				double elapsed = now - timestamp;

				if(elapsed >= frame_duration)
				{
					timestamp = now;
					uint32_t * frame = xaf_next(ctx->xaf);
					if(frame)
					{
						int sstride = surface_get_stride(ctx->surface);
						int fstride = width * 4;
						if(sstride == fstride)
						{
							xos_memcpy(surface_get_pixels(ctx->surface), frame, fstride * height);
						}
						else
						{
							for(uint32_t y = 0; y < height; y++)
								xos_memcpy(surface_get_pixels(ctx->surface) + y * sstride, (uint8_t *)frame + y * fstride, fstride);
						}

						window_dirtylist_fullscreen(ctx->window);
						{
							surface_clear(window_get_surface(ctx->window), &color, 0, 0, 0, 0);
							surface_blit(window_get_surface(ctx->window), NULL, &ctx->matrix, ctx->surface);
							if(!hide)
							{
								int w = window_get_width(ctx->window) - 20;
								int p = XCLAMP((int)(w * ((double)(xaf_get_findex(ctx->xaf) + 1) / (double)xaf_get_nframes(ctx->xaf))), 1, w);
								surface_clear(window_get_surface(ctx->window), &(struct color_t){0xf5, 0xf5, 0xf5, 0xff}, 10, window_get_height(ctx->window) - 10, p, 2);
								if(w > p)
									surface_clear(window_get_surface(ctx->window), &(struct color_t){0x85, 0x85, 0x85, 0xff}, 10 + p, window_get_height(ctx->window) - 10, w - p, 2);
							}
						}
						window_present_commit(ctx->window);
					}
					else
					{
						if(loop)
							xaf_reset(ctx->xaf);
						else
							ctx->quit = 1;
					}
				}
			}
		}
		xaf_context_free(ctx);
	}
	return 0;
}

static struct command_t cmd_xaf = {
	.name	= "xaf",
	.desc	= "video player for xaf (x animation format) file",
	.usage	= usage,
	.exec	= do_xaf,
};

static void xaf_cmd_init(void)
{
	register_command(&cmd_xaf);
}

static void xaf_cmd_exit(void)
{
	unregister_command(&cmd_xaf);
}

command_initcall(xaf_cmd_init);
command_exitcall(xaf_cmd_exit);
