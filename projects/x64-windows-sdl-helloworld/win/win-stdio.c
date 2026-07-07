#include <win/win.h>

ssize_t win_stdio_read(void * buf, size_t count)
{
	ssize_t i;
	char * p;

	win_event_sdl_poll();
	for(i = 0, p = buf; i < count; i++)
	{
		if(_kbhit())
			p[i] = _getch();
		else
			break;
	}
	return i;
}

ssize_t win_stdio_write(void * buf, size_t count)
{
	ssize_t i;
	char * p;

	for(i = 0, p = buf; i < count; i++)
		putchar(p[i]);
	return i;
}
