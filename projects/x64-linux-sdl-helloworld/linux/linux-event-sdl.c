#include <linux/linux.h>

struct linux_event_callback_t {
	struct {
		void * device;
		void (*down)(void * device, unsigned int key);
		void (*up)(void * device, unsigned int key);
	} key;

	struct {
		void * device;
		void (*down)(void * device, int x, int y, unsigned int button);
		void (*move)(void * device, int x, int y);
		void (*up)(void * device, int x, int y, unsigned int button);
		void (*wheel)(void * device, int dx, int dy);
	} mouse;

	struct {
		void * device;
		void (*begin)(void * device, int x, int y, unsigned int id);
		void (*move)(void * device, int x, int y, unsigned int id);
		void (*end)(void * device, int x, int y, unsigned int id);
	} touch;

	struct {
		void * device;
		void (*left_stick)(void * device, int x, int y);
		void (*right_stick)(void * device, int x, int y);
		void (*left_trigger)(void * device, int v);
		void (*right_trigger)(void * device, int v);
		void (*button_down)(void * device, unsigned int button);
		void (*button_up)(void * device, unsigned int button);
	} joystick;
};

struct linux_event_sdl_context_t {
	struct linux_event_callback_t cb;
	SDL_Thread * thread;
};

static unsigned int keycode_map(SDL_Keycode code)
{
	unsigned int key;

	switch(code)
	{
	case SDLK_POWER:      key = 1; break;
	case SDLK_UP:         key = 2; break;
	case SDLK_DOWN:       key = 3; break;
	case SDLK_LEFT:       key = 4; break;
	case SDLK_RIGHT:      key = 5; break;
	case SDLK_VOLUMEUP:   key = 6; break;
	case SDLK_VOLUMEDOWN: key = 7; break;
	case SDLK_MUTE:       key = 8; break;
	case SDLK_TAB:        key = 9; break;
	case SDLK_ESCAPE:     key = 10; break;
	case SDLK_BACKSPACE:  key = 11; break;
	case SDLK_PAUSE:      key = 12; break;
	case SDLK_RETURN:     key = 13; break;
	case SDLK_LCTRL:      key = 14; break;
	case SDLK_RCTRL:      key = 15; break;
	case SDLK_LALT:       key = 16; break;
	case SDLK_RALT:       key = 17; break;
	case SDLK_LSHIFT:     key = 18; break;
	case SDLK_RSHIFT:     key = 19; break;

	case SDLK_F1:  key = 20; break;
	case SDLK_F2:  key = 21; break;
	case SDLK_F3:  key = 22; break;
	case SDLK_F4:  key = 23; break;
	case SDLK_F5:  key = 24; break;
	case SDLK_F6:  key = 25; break;
	case SDLK_F7:  key = 26; break;
	case SDLK_F8:  key = 27; break;
	case SDLK_F9:  key = 28; break;
	case SDLK_F10: key = 29; break;
	case SDLK_F11: key = 30; break;
	case SDLK_F12: key = 31; break;

	default:
		key = code;
		break;
	}
	return key;
}

static int linux_event_sdl_thread(void * data)
{
	struct linux_event_sdl_context_t * ctx = (struct linux_event_sdl_context_t *)data;
	struct linux_event_callback_t * cb = &ctx->cb;
	SDL_Event e;
	unsigned int button;
	int width, height;

	while(1)
	{
		if(SDL_WaitEvent(&e))
		{
			do {
				switch(e.type)
				{
				case SDL_EVENT_KEY_DOWN:
					if(cb->key.down)
						cb->key.down(cb->key.device, keycode_map(e.key.key));
					break;

				case SDL_EVENT_KEY_UP:
					if(cb->key.up)
						cb->key.up(cb->key.device, keycode_map(e.key.key));
					break;

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					switch(e.button.button)
					{
					case SDL_BUTTON_LEFT:
						button = (0x1 << 0);
						break;
					case SDL_BUTTON_MIDDLE:
						button = (0x1 << 2);
						break;
					case SDL_BUTTON_RIGHT:
						button = (0x1 << 1);
						break;
					case SDL_BUTTON_X1:
						button = (0x1 << 3);
						break;
					case SDL_BUTTON_X2:
						button = (0x1 << 4);
						break;
					default:
						button = 0x00;
						break;
					}
					if(cb->mouse.down && (button != 0x00))
						cb->mouse.down(cb->mouse.device, e.button.x, e.button.y, button);
					break;

				case SDL_EVENT_MOUSE_MOTION:
					if(cb->mouse.move)
						cb->mouse.move(cb->mouse.device, e.motion.x, e.motion.y);
					break;

				case SDL_EVENT_MOUSE_BUTTON_UP:
					switch(e.button.button)
					{
					case SDL_BUTTON_LEFT:
						button = (0x1 << 0);
						break;
					case SDL_BUTTON_MIDDLE:
						button = (0x1 << 2);
						break;
					case SDL_BUTTON_RIGHT:
						button = (0x1 << 1);
						break;
					case SDL_BUTTON_X1:
						button = (0x1 << 3);
						break;
					case SDL_BUTTON_X2:
						button = (0x1 << 4);
						break;
					default:
						button = 0x00;
						break;
					}
					if(cb->mouse.up && (button != 0x00))
						cb->mouse.up(cb->mouse.device, e.button.x, e.button.y, button);
					break;

				case SDL_EVENT_MOUSE_WHEEL:
					if(cb->mouse.wheel)
						cb->mouse.wheel(cb->mouse.device, e.wheel.x, e.wheel.y);
					break;

				case SDL_EVENT_FINGER_DOWN:
					if(cb->touch.begin)
					{
						SDL_GetWindowSizeInPixels(SDL_GetWindowFromID(e.tfinger.windowID), &width, &height);
						cb->touch.begin(cb->touch.device, (int)(e.tfinger.x * width), (int)(e.tfinger.y * height), (unsigned int)e.tfinger.fingerID);
					}
					break;

				case SDL_EVENT_FINGER_MOTION:
					if(cb->touch.move)
					{
						SDL_GetWindowSizeInPixels(SDL_GetWindowFromID(e.tfinger.windowID), &width, &height);
						cb->touch.move(cb->touch.device, (int)(e.tfinger.x * width), (int)(e.tfinger.y * height), (unsigned int)e.tfinger.fingerID);
					}
					break;

				case SDL_EVENT_FINGER_UP:
					if(cb->touch.end)
					{
						SDL_GetWindowSizeInPixels(SDL_GetWindowFromID(e.tfinger.windowID), &width, &height);
						cb->touch.end(cb->touch.device, (int)(e.tfinger.x * width), (int)(e.tfinger.y * height), (unsigned int)e.tfinger.fingerID);
					}
					break;

				case SDL_EVENT_GAMEPAD_ADDED:
					break;

				case SDL_EVENT_GAMEPAD_REMOVED:
					break;

				case SDL_EVENT_GAMEPAD_AXIS_MOTION:
					switch(e.jaxis.axis)
					{
					case SDL_GAMEPAD_AXIS_LEFTX:
					case SDL_GAMEPAD_AXIS_LEFTY:
						break;

					case SDL_GAMEPAD_AXIS_RIGHTX:
					case SDL_GAMEPAD_AXIS_RIGHTY:
						break;

					case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
						break;

					case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
						break;

					default:
						break;
					}
					break;

				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
					switch(e.button.button)
					{
					case SDL_GAMEPAD_BUTTON_SOUTH:
						button = (0x1 << 4);
						break;
					case SDL_GAMEPAD_BUTTON_EAST:
						button = (0x1 << 5);
						break;
					case SDL_GAMEPAD_BUTTON_WEST:
						button = (0x1 << 6);
						break;
					case SDL_GAMEPAD_BUTTON_NORTH:
						button = (0x1 << 7);
						break;
					case SDL_GAMEPAD_BUTTON_BACK:
						button = (0x1 << 8);
						break;
					case SDL_GAMEPAD_BUTTON_GUIDE:
						button = (0x1 << 9);
						break;
					case SDL_GAMEPAD_BUTTON_START:
						button = (0x1 << 10);
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_STICK:
						button = (0x1 << 11);
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
						button = (0x1 << 12);
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
						button = (0x1 << 13);
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
						button = (0x1 << 14);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_UP:
						button = (0x1 << 0);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
						button = (0x1 << 1);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
						button = (0x1 << 2);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
						button = (0x1 << 3);
						break;
					default:
						button = 0x00;
						break;
					}
					if(cb->joystick.button_down && (button != 0x00))
						cb->joystick.button_down(cb->joystick.device, button);
					break;

				case SDL_EVENT_GAMEPAD_BUTTON_UP:
					switch(e.button.button)
					{
					case SDL_GAMEPAD_BUTTON_SOUTH:
						button = (0x1 << 4);
						break;
					case SDL_GAMEPAD_BUTTON_EAST:
						button = (0x1 << 5);
						break;
					case SDL_GAMEPAD_BUTTON_WEST:
						button = (0x1 << 6);
						break;
					case SDL_GAMEPAD_BUTTON_NORTH:
						button = (0x1 << 7);
						break;
					case SDL_GAMEPAD_BUTTON_BACK:
						button = (0x1 << 8);
						break;
					case SDL_GAMEPAD_BUTTON_GUIDE:
						button = (0x1 << 9);
						break;
					case SDL_GAMEPAD_BUTTON_START:
						button = (0x1 << 10);
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_STICK:
						button = (0x1 << 11);
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
						button = (0x1 << 12);
						break;
					case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
						button = (0x1 << 13);
						break;
					case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
						button = (0x1 << 14);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_UP:
						button = (0x1 << 0);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
						button = (0x1 << 1);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
						button = (0x1 << 2);
						break;
					case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
						button = (0x1 << 3);
						break;
					default:
						button = 0x00;
						break;
					}
					if(cb->joystick.button_up && (button != 0x00))
						cb->joystick.button_up(cb->joystick.device, button);
					break;

				case SDL_EVENT_QUIT:
					linux_exit();
					break;

				default:
					break;
				}
			} while(SDL_PollEvent(&e));
		}
	}
	return 0;
}

void * linux_event_sdl_open(void)
{
	struct linux_event_sdl_context_t * ctx;

	ctx = malloc(sizeof(struct linux_event_sdl_context_t));
	if(!ctx)
		return NULL;

	memset(ctx, 0, sizeof(struct linux_event_sdl_context_t));
	ctx->thread = SDL_CreateThread(linux_event_sdl_thread, "event", ctx);
	return ctx;
}

void linux_event_sdl_close(void * context)
{
	struct linux_event_sdl_context_t * ctx = (struct linux_event_sdl_context_t *)context;
	if(ctx)
	{
		SDL_WaitThread(ctx->thread, NULL);
		free(ctx);
	}
}

void linux_event_sdl_mouse_set_range(void * context, int xmax, int ymax)
{
}

void linux_event_sdl_mouse_get_range(void * context, int * xmax, int * ymax)
{
}

void linux_event_sdl_mouse_set_sensitivity(void * context, int s)
{
}

void linux_event_sdl_mouse_get_sensitivity(void * context, int * s)
{
}

void linux_event_sdl_set_key_callback(void * context, void * device,
		void (*down)(void * device, unsigned int key),
		void (*up)(void * device, unsigned int key))
{
	struct linux_event_sdl_context_t * ctx = (struct linux_event_sdl_context_t *)context;
	if(ctx)
	{
		ctx->cb.key.device = device;
		ctx->cb.key.down = down;
		ctx->cb.key.up = up;
	}
}

void linux_event_sdl_set_mouse_callback(void * context, void * device,
		void (*down)(void * device, int x, int y, unsigned int button),
		void (*move)(void * device, int x, int y),
		void (*up)(void * device, int x, int y, unsigned int button),
		void (*wheel)(void * device, int dx, int dy))
{
	struct linux_event_sdl_context_t * ctx = (struct linux_event_sdl_context_t *)context;
	if(ctx)
	{
		ctx->cb.mouse.device = device;
		ctx->cb.mouse.down = down;
		ctx->cb.mouse.move = move;
		ctx->cb.mouse.up = up;
		ctx->cb.mouse.wheel = wheel;
	}
}

void linux_event_sdl_set_touch_callback(void * context, void * device,
		void (*begin)(void * device, int x, int y, unsigned int id),
		void (*move)(void * device, int x, int y, unsigned int id),
		void (*end)(void * device, int x, int y, unsigned int id))
{
	struct linux_event_sdl_context_t * ctx = (struct linux_event_sdl_context_t *)context;
	if(ctx)
	{
		ctx->cb.touch.device = device;
		ctx->cb.touch.begin = begin;
		ctx->cb.touch.move = move;
		ctx->cb.touch.end = end;
	}
}

void linux_event_sdl_set_joystick_callback(void * context, void * device,
		void (*left_stick)(void * device, int x, int y),
		void (*right_stick)(void * device, int x, int y),
		void (*left_trigger)(void * device, int v),
		void (*right_trigger)(void * device, int v),
		void (*button_down)(void * device, unsigned int button),
		void (*button_up)(void * device, unsigned int button))
{
	struct linux_event_sdl_context_t * ctx = (struct linux_event_sdl_context_t *)context;
	if(ctx)
	{
		ctx->cb.joystick.device = device;
		ctx->cb.joystick.left_stick = left_stick;
		ctx->cb.joystick.right_stick = right_stick;
		ctx->cb.joystick.left_trigger = left_trigger;
		ctx->cb.joystick.right_trigger = right_trigger;
		ctx->cb.joystick.button_down = button_down;
		ctx->cb.joystick.button_up = button_up;
	}
}
