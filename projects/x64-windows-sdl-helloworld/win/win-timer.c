#include <win/win.h>

struct win_timer_context_t {
	void (*cb)(void *);
	void * data;
};

static Uint64 timer_callback(void * userdata, SDL_TimerID tid, Uint64 interval)
{
	struct win_timer_context_t * tctx = (struct win_timer_context_t *)(userdata);

	if(tctx->cb)
		tctx->cb(tctx->data);
	return 0;
}

void win_timer_init(void)
{
}

void win_timer_exit(void)
{
}

void win_timer_next(uint64_t time, void (*cb)(void *), void * data)
{
	static struct win_timer_context_t tctx;

	tctx.cb = cb;
	tctx.data = data;
	SDL_AddTimerNS(time, timer_callback, &tctx);
}

uint64_t win_timer_count(void)
{
	win_event_sdl_poll();
	return (uint64_t)SDL_GetPerformanceCounter();
}

uint64_t win_timer_frequency(void)
{
	return (uint64_t)SDL_GetPerformanceFrequency();
}

uint64_t win_realtime(void)
{
	SDL_Time t;

	if(SDL_GetCurrentTime(&t))
		return (uint64_t)t;
	return 0;
}
