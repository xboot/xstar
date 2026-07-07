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
#include <xnes.h>
#include <kernel/command/command.h>

struct window_context_t {
	struct window_t * window;
	struct xnes_ctx_t * nes;
	struct xnes_state_t * state;
	int rewind;
	ktime_t timestamp;
	ktime_t elapsed;
};

static struct window_context_t * window_context_alloc(const char * fb, const char * input)
{
	struct window_context_t * wctx;

	wctx = xos_mem_malloc(sizeof(struct window_context_t));
	if(!wctx)
		return NULL;

	wctx->window = window_alloc(fb, input, -1);
	wctx->nes = NULL;
	wctx->state = NULL;
	wctx->rewind = 0;
	wctx->timestamp = ktime_get();
	wctx->elapsed = ns_to_ktime(0);

	return wctx;
}

static void window_context_free(struct window_context_t * wctx)
{
	if(wctx)
	{
		if(wctx->nes)
			xnes_ctx_free(wctx->nes);
		if(wctx->state)
			xnes_state_free(wctx->state);
		if(wctx->window)
			window_free(wctx->window);
		xos_mem_free(wctx);
	}
}

static void window_context_reload(struct window_context_t * wctx, const char * filename)
{
	char fpath[CONFIG_XSTAR_MAX_PATH];

	if(wctx)
	{
		if(wctx->nes)
		{
			xnes_ctx_free(wctx->nes);
			wctx->nes = NULL;
		}
		if(shell_realpath(filename, fpath) >= 0)
		{
			struct xfs_file_t * file = xfs_open_read(shell_getxfs(), fpath);
			if(file)
			{
				int64_t len = xfs_length(file);
				if(len > 0)
				{
					void * buf = xos_mem_malloc(len);
					if(buf)
					{
						len = xfs_read(file, buf, len);
						wctx->nes = xnes_ctx_alloc(buf, len);
						if(wctx->nes)
						{
							if(wctx->state)
								xnes_state_free(wctx->state);
							wctx->state = xnes_state_alloc(wctx->nes, 60 * 10);
						}
						xos_mem_free(buf);
					}
				}
				xfs_close(file);
			}
		}
	}
}

static void window_context_screen_refresh(struct window_context_t * wctx)
{
	window_dirtylist_fullscreen(wctx->window);
	window_present_clear(wctx->window);
	{
		for(int y = 0; y < 240; y++)
		{
			for(int x = 0; x < 256; x++)
			{
				uint32_t c = xnes_get_pixel(wctx->nes, x, y);
				struct color_t col;
				color_set_premult(&col, c);
				surface_set_pixel(wctx->window->surface, x, y, &col);
			}
		}
	}
	window_present_commit(wctx->window);
}

static void window_context_update(struct window_context_t * wctx)
{
	if(wctx->nes)
	{
		if(wctx->rewind)
		{
			if(ktime_after(ktime_sub(ktime_get(), wctx->timestamp), wctx->elapsed))
			{
				wctx->timestamp = ktime_get();
				wctx->elapsed = ns_to_ktime(16666666);
				xnes_state_pop(wctx->state);
				window_context_screen_refresh(wctx);
			}
			else
				mdelay(1);
		}
		else
		{
			if(ktime_after(ktime_sub(ktime_get(), wctx->timestamp), wctx->elapsed))
			{
				wctx->timestamp = ktime_get();
				wctx->elapsed = ns_to_ktime(xnes_step_frame(wctx->nes));
				window_context_screen_refresh(wctx);
				xnes_state_push(wctx->state);
			}
			else
				mdelay(1);
		}
	}
	else
		mdelay(1);
}

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    xnes <rom> [framebuffer] [input]\r\n");
}

static int do_xnes(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 1, 3))
	{
		usage();
		return -1;
	}

	const char * rom = sarg_at(&sarg, 0);
	const char * fb = sarg_at(&sarg, 1);
	const char * input = sarg_at(&sarg, 2);
	struct window_context_t * wctx = window_context_alloc(fb, input);
	if(wctx)
	{
		window_context_reload(wctx, rom);
		int done = 0;
		while(!done)
		{
			struct event_t e;
			if(window_pump_event(wctx->window, &e))
			{
				if(wctx->nes)
				{
					switch(e.type)
					{
					case EVENT_TYPE_KEY_DOWN:
						switch(e.e.key_down.key)
						{
						case KB_KEY_HOME:
							xnes_reset(wctx->nes);
							break;
						case KB_KEY_F1:
							xnes_set_speed(wctx->nes, 0.5);
							break;
						case KB_KEY_F2:
							xnes_set_speed(wctx->nes, 2.0);
							break;
						case KB_KEY_BACK:
							wctx->rewind = 1;
							break;
						case KB_KEY_UP:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_UP, 0);
							break;
						case KB_KEY_DOWN:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_DOWN, 0);
							break;
						case KB_KEY_LEFT:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_LEFT, 0);
							break;
						case KB_KEY_RIGHT:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_RIGHT, 0);
							break;
						case KB_KEY_X:
						case KB_KEY_x:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
							break;
						case KB_KEY_Z:
						case KB_KEY_z:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
							break;
						case KB_KEY_MENU:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_SELECT, 0);
							break;
						case KB_KEY_ENTER:
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_START, 0);
							break;
						default:
							break;
						}
						break;

					case EVENT_TYPE_KEY_UP:
						switch(e.e.key_up.key)
						{
						case KB_KEY_HOME:
							break;
						case KB_KEY_F1:
							xnes_set_speed(wctx->nes, 1.0);
							break;
						case KB_KEY_F2:
							xnes_set_speed(wctx->nes, 1.0);
							break;
						case KB_KEY_BACK:
							wctx->rewind = 0;
							break;
						case KB_KEY_UP:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_UP);
							break;
						case KB_KEY_DOWN:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_DOWN);
							break;
						case KB_KEY_LEFT:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_LEFT);
							break;
						case KB_KEY_RIGHT:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_RIGHT);
							break;
						case KB_KEY_X:
						case KB_KEY_x:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
							break;
						case KB_KEY_Z:
						case KB_KEY_z:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
							break;
						case KB_KEY_MENU:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_SELECT);
							break;
						case KB_KEY_ENTER:
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_START);
							break;
						default:
							break;
						}
						break;

					case EVENT_TYPE_JOYSTICK_BUTTONDOWN:
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_UP)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_UP, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_DOWN)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_DOWN, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_LEFT)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_LEFT, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_RIGHT)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_RIGHT, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_A)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_B)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_X)
							xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, XNES_JOYSTICK_A, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_Y)
							xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, XNES_JOYSTICK_B, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_BACK)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_SELECT, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_START)
							xnes_controller_joystick_p1(&wctx->nes->ctl, XNES_JOYSTICK_START, 0);
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_LBUMPER)
							wctx->rewind = 1;
						if(e.e.joystick_button_down.button & JOYSTICK_BUTTON_RBUMPER)
							xnes_set_speed(wctx->nes, 0.5);
						break;

					case EVENT_TYPE_JOYSTICK_BUTTONUP:
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_UP)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_UP);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_DOWN)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_DOWN);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_LEFT)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_LEFT);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_RIGHT)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_RIGHT);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_A)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_B)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_X)
							xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, 0, XNES_JOYSTICK_A);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_Y)
							xnes_controller_joystick_p1_turbo(&wctx->nes->ctl, 0, XNES_JOYSTICK_B);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_BACK)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_SELECT);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_START)
							xnes_controller_joystick_p1(&wctx->nes->ctl, 0, XNES_JOYSTICK_START);
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_LBUMPER)
							wctx->rewind = 0;
						if(e.e.joystick_button_up.button & JOYSTICK_BUTTON_RBUMPER)
							xnes_set_speed(wctx->nes, 1.0);
						break;

					default:
						break;
					}
				}
			}
			window_context_update(wctx);
			if(shell_ctrlc())
				done = 1;
		}
		window_context_free(wctx);
	}
	return 0;
}

static struct command_t cmd_xnes = {
	.name	= "xnes",
	.desc	= "the nintendo entertainment system emulator",
	.usage	= usage,
	.exec	= do_xnes,
};

static void xnes_cmd_init(void)
{
	register_command(&cmd_xnes);
}

static void xnes_cmd_exit(void)
{
	unregister_command(&cmd_xnes);
}

command_initcall(xnes_cmd_init);
command_exitcall(xnes_cmd_exit);
