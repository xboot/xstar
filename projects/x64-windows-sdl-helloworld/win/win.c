#include <win/win.h>

void win_init(void)
{
	if(AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
	{
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		freopen("CONIN$", "r", stdin);
		SetConsoleOutputCP(65001);
		SetConsoleCP(65001);
	}
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS | SDL_INIT_SENSOR | SDL_INIT_CAMERA);
}

void win_exit(void)
{
	SDL_Quit();
	exit(0);
}
