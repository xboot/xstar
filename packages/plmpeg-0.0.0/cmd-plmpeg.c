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
#include <kernel/command/command.h>

/*
 * ffmpeg -i input.mp4 -c:v mpeg1video -q:v 0 -c:a libtwolame -b:a 224k -format mpeg output.mpg
 */
#define PL_MPEG_IMPLEMENTATION
#define PLM_NO_STDIO
#include <plmpeg.h>

enum plmpeg_diplay_mode_t {
	PLMPEG_DISPLAY_MODE_NONE	= 0,
	PLMPEG_DISPLAY_MODE_CONTAIN	= 1,
	PLMPEG_DISPLAY_MODE_COVER	= 2,
	PLMPEG_DISPLAY_MODE_FILL	= 3,
};

struct plmpeg_context_t {
	struct window_t * window;
	struct surface_t * surface;
	struct matrix2d_t matrix;
	struct color_t color;
	struct audio_sink_t * sink;
	struct audio_frame_t af;
	char * playback;
	plm_t * plm;
	double timestamp;
	int hide;
	int quit;
};

static struct plmpeg_context_t * plmpeg_context_alloc(const char * fb, const char * input)
{
	struct plmpeg_context_t * ctx;

	ctx = xos_mem_malloc(sizeof(struct plmpeg_context_t));
	if(!ctx)
		return ctx;

	ctx->window = window_alloc(fb, input, -1);
	ctx->surface = NULL;
	ctx->sink = NULL;
	ctx->playback = NULL;
	ctx->plm = NULL;
	ctx->timestamp = 0;
	ctx->hide = 0;
	ctx->quit = 0;

	return ctx;
}

static void plmpeg_context_free(struct plmpeg_context_t * ctx)
{
	if(ctx)
	{
		if(ctx->plm)
			plm_destroy(ctx->plm);
		if(ctx->playback)
			xos_mem_free(ctx->playback);
		if(ctx->sink)
			audio_sink_free(ctx->sink);
		if(ctx->surface)
			surface_free(ctx->surface);
		if(ctx->window)
			window_free(ctx->window);
		xos_mem_free(ctx);
	}
}

static void plmpeg_on_video(plm_t * player, plm_frame_t * frame, void * user)
{
	struct plmpeg_context_t * ctx = (struct plmpeg_context_t *)user;
	plm_frame_to_bgra(frame, surface_get_pixels(ctx->surface), surface_get_stride(ctx->surface));

	window_dirtylist_fullscreen(ctx->window);
	{
		surface_clear(window_get_surface(ctx->window), &ctx->color, 0, 0, 0, 0);
		surface_blit(window_get_surface(ctx->window), NULL, &ctx->matrix, ctx->surface);
		if(!ctx->hide)
		{
			int w = window_get_width(ctx->window) - 20;
			int p = XCLAMP((int)(w * (plm_get_time(ctx->plm) / plm_get_duration(ctx->plm))), 1, w);
			surface_clear(window_get_surface(ctx->window), &(struct color_t){0xf5, 0xf5, 0xf5, 0xff}, 10, window_get_height(ctx->window) - 10, p, 2);
			if(w > p)
				surface_clear(window_get_surface(ctx->window), &(struct color_t){0x85, 0x85, 0x85, 0xff}, 10 + p, window_get_height(ctx->window) - 10, w - p, 2);
		}
	}
	window_present_commit(ctx->window);
}

static void plmpeg_on_audio(plm_t * player, plm_samples_t * samples, void * user)
{
	struct plmpeg_context_t * ctx = (struct plmpeg_context_t *)user;

	if(ctx->sink)
	{
		ctx->af.frames = samples->count;
		ctx->af.samples = samples->interleaved;
		audio_sink_write(ctx->sink, &ctx->af);
	}
}

static int plmpeg_reload(struct plmpeg_context_t * ctx, const char * filename)
{
	if(ctx && filename)
	{
		if(ctx->plm)
		{
			plm_destroy(ctx->plm);
			ctx->plm = NULL;
		}
		ctx->plm = plm_create_with_xfs_filename(shell_getxfs(), filename);
		if(ctx->plm)
		{
			ctx->surface = surface_alloc(plm_get_width(ctx->plm), plm_get_height(ctx->plm));
			if(ctx->surface)
				surface_clear(ctx->surface, &(struct color_t){0x00, 0x00, 0x00, 0xff}, 0, 0, 0, 0);
			plm_set_video_decode_callback(ctx->plm, plmpeg_on_video, ctx);
			plm_set_audio_decode_callback(ctx->plm, plmpeg_on_audio, ctx);
			plm_set_audio_enabled(ctx->plm, 1);
			plm_set_audio_stream(ctx->plm, 0);
			if(plm_get_num_audio_streams(ctx->plm) > 0)
			{
				int samplerate = plm_get_samplerate(ctx->plm);
				if(ctx->sink)
					audio_sink_free(ctx->sink);
				ctx->sink = audio_sink_alloc_from_playback(ctx->playback, samplerate, 2);
				if(ctx->sink)
				{
					ctx->af.rate = samplerate;
					ctx->af.channel = 2;
					ctx->af.frames = 0;
					ctx->af.samples = NULL;
				}
			}
			return 1;
		}
	}
	return 0;
}

static void plmpeg_set_display_mode(struct plmpeg_context_t * ctx, enum plmpeg_diplay_mode_t mode)
{
	if(ctx && ctx->window && ctx->surface)
	{
		float sx = 1.0, sy = 1.0;
		switch(mode)
		{
		case PLMPEG_DISPLAY_MODE_NONE:
			sx = 1.0;
			sy = 1.0;
			break;
		case PLMPEG_DISPLAY_MODE_CONTAIN:
			sx = (float)window_get_width(ctx->window) / (float)surface_get_width(ctx->surface);
			sy = (float)window_get_height(ctx->window) / (float)surface_get_height(ctx->surface);
			if(sx >= sy)
				sx = sy;
			else
				sy = sx;
			break;
		case PLMPEG_DISPLAY_MODE_COVER:
			sx = (float)window_get_width(ctx->window) / (float)surface_get_width(ctx->surface);
			sy = (float)window_get_height(ctx->window) / (float)surface_get_height(ctx->surface);
			if(sx <= sy)
				sx = sy;
			else
				sy = sx;
			break;
		case PLMPEG_DISPLAY_MODE_FILL:
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
	shell_printf("    plmpeg <file> [-m=none|contain|cover|fill] [-c=color] [-f=framebuffer] [-i=input] [-p=playback] [-lock] [-hide] [-loop]\r\n");
}

static int do_plmpeg(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char *[]){ "-m", "-c", "-f", "-i", "-p", "-lock", "-hide", "-loop", NULL }, 0, 1))
	{
		usage();
		return -1;
	}

	char fpath[CONFIG_XSTAR_MAX_PATH];
	const char * filename = sarg_at(&sarg, 0);
	const char * fb = sarg_get(&sarg, "-f", NULL);
	const char * input = sarg_get(&sarg, "-i", NULL);
	const char * playback = sarg_get(&sarg, "-p", NULL);
	int lock = sarg_has(&sarg, "-lock");
	int hide = sarg_has(&sarg, "-hide");
	int loop = sarg_has(&sarg, "-loop");
	enum plmpeg_diplay_mode_t mode = PLMPEG_DISPLAY_MODE_NONE;
	const char * m = sarg_get(&sarg, "-m", NULL);
	if(m)
	{
		switch(shash(m))
		{
		case 0x7c9b47f5: /* "none" */
			mode = PLMPEG_DISPLAY_MODE_NONE;
			break;
		case 0xd37987d1: /* "contain" */
			mode = PLMPEG_DISPLAY_MODE_CONTAIN;
			break;
		case 0x0f3d5b84: /* "cover" */
			mode = PLMPEG_DISPLAY_MODE_COVER;
			break;
		case 0x7c96cb2c: /* "fill" */
			mode = PLMPEG_DISPLAY_MODE_FILL;
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
	struct plmpeg_context_t * ctx = plmpeg_context_alloc(fb, input);
	if(ctx)
	{
		xos_memcpy(&ctx->color, &color, sizeof(struct color_t));
		ctx->playback = xos_strdup(playback);
		ctx->hide = hide;
		if(plmpeg_reload(ctx, fpath))
		{
			plm_set_loop(ctx->plm, loop);
			plmpeg_set_display_mode(ctx, mode);
			if(ctx->plm)
			{
				struct event_t e;
				int pressed = 0;
				while(!ctx->quit)
				{
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

							case KB_KEY_LEFT:
								plm_seek(ctx->plm, plm_get_time(ctx->plm) - 3, 0);
								break;

							case KB_KEY_RIGHT:
								plm_seek(ctx->plm, plm_get_time(ctx->plm) + 3, 0);
								break;

							case KB_KEY_UP:
							case KB_KEY_VOLUME_UP:
								audio_sink_set_volume(ctx->sink, audio_sink_get_volume(ctx->sink) + 100);
								break;

							case KB_KEY_DOWN:
							case KB_KEY_VOLUME_DOWN:
								audio_sink_set_volume(ctx->sink, audio_sink_get_volume(ctx->sink) - 100);
								break;

							case KB_KEY_SPACE:
								mode = (mode + 1) & 0x3;
								plmpeg_set_display_mode(ctx, mode);
								break;

							default:
								break;
							}
							break;
						case EVENT_TYPE_KEY_UP:
							break;

						case EVENT_TYPE_MOUSE_DOWN:
							if(e.e.mouse_down.button & MOUSE_BUTTON_LEFT)
								pressed = 1;
							break;
						case EVENT_TYPE_MOUSE_MOVE:
							if(pressed)
								plm_seek(ctx->plm, plm_get_duration(ctx->plm) * ((double)e.e.mouse_move.x / (double)window_get_width(ctx->window)), 0);
							break;
						case EVENT_TYPE_MOUSE_UP:
							pressed = 0;
							break;

						case EVENT_TYPE_SYSTEM_EXIT:
							ctx->quit = 1;
							break;

						default:
							break;
						}
					}

					double now = (double)ktime_to_ns(ktime_get()) / 1000000000.0;
					double elapsed = now - ctx->timestamp;
					ctx->timestamp = now;
					if(elapsed > 0.0)
					{
						if(elapsed > 1.0 / 30.0)
							elapsed = 1.0 / 30.0;
						plm_decode(ctx->plm, elapsed);
					}

					if(plm_has_ended(ctx->plm))
						ctx->quit = 1;
				}
			}
		}
		plmpeg_context_free(ctx);
	}
	return 0;
}

static struct command_t cmd_plmpeg = {
	.name	= "plmpeg",
	.desc	= "video player for mpeg1(vcd) file",
	.usage	= usage,
	.exec	= do_plmpeg,
};

static void plmpeg_cmd_init(void)
{
	register_command(&cmd_plmpeg);
}

static void plmpeg_cmd_exit(void)
{
	unregister_command(&cmd_plmpeg);
}

command_initcall(plmpeg_cmd_init);
command_exitcall(plmpeg_cmd_exit);
