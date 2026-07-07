#include <win/win.h>

struct win_fb_sdl_context_t {
	SDL_Window * window;
	SDL_Surface * screen;
	int width, height;
	int pwidth, pheight;
};

void * win_fb_sdl_open(const char * title, int width, int height)
{
	struct win_fb_sdl_context_t * ctx;
	Uint32 flags = SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;

	ctx = win_mem_malloc(sizeof(struct win_fb_sdl_context_t));
	if(!ctx)
		return NULL;

	ctx->window = SDL_CreateWindow(title ? title : "Xboot Runtime Environment", width, height, flags);
	if(!ctx->window)
	{
		win_mem_free(ctx);
		return NULL;
	}

	ctx->screen = SDL_GetWindowSurface(ctx->window);
	if(!ctx->screen)
	{
		if(ctx->window)
			SDL_DestroyWindow(ctx->window);
		win_mem_free(ctx);
		return NULL;
	}

	float dpi = SDL_GetWindowPixelDensity(ctx->window);
	SDL_GetWindowSizeInPixels(ctx->window, &ctx->width, &ctx->height);
	ctx->pwidth = (int)(25.4 * ctx->width / dpi);
	ctx->pheight = (int)(25.4 * ctx->height / dpi);

	return ctx;
}

void win_fb_sdl_close(void * context)
{
	struct win_fb_sdl_context_t * ctx = (struct win_fb_sdl_context_t *)context;

	if(!ctx)
		return;
	if(ctx->window)
		SDL_DestroyWindow(ctx->window);
	if(ctx->screen)
		SDL_DestroySurface(ctx->screen);
	win_mem_free(ctx);
}

int win_fb_sdl_get_width(void * context)
{
	struct win_fb_sdl_context_t * ctx = (struct win_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->width;
	return 0;
}

int win_fb_sdl_get_height(void * context)
{
	struct win_fb_sdl_context_t * ctx = (struct win_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->height;
	return 0;
}

int win_fb_sdl_get_pwidth(void * context)
{
	struct win_fb_sdl_context_t * ctx = (struct win_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->pwidth;
	return 0;
}

int win_fb_sdl_get_pheight(void * context)
{
	struct win_fb_sdl_context_t * ctx = (struct win_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->pheight;
	return 0;
}

int win_fb_sdl_surface_create(void * context, struct win_fb_surface_t * surface, int width, int height)
{
	SDL_Surface * face = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
	if(!face)
		return 0;

	surface->width = face->w;
	surface->height = face->h;
	surface->stride = face->pitch;
	surface->pixlen = face->h * face->pitch;
	surface->pixels = face->pixels;
	surface->priv = face;

	return 1;
}

int win_fb_sdl_surface_destroy(void * context, struct win_fb_surface_t * surface)
{
	if(surface)
		SDL_DestroySurface(surface->priv);
	return 1;
}

int win_fb_sdl_surface_present(void * context, struct win_fb_surface_t * surface, struct win_dirtylist_t * l)
{
	struct win_fb_sdl_context_t * ctx = (struct win_fb_sdl_context_t *)context;
	struct win_region_t * r;
	int i;

	win_event_sdl_poll();
	if(l && (l->count > 0))
	{
		for(i = 0; i < l->count; i++)
		{
			r = &l->items[i].region;
			SDL_BlitSurface(surface->priv, (SDL_Rect *)r, ctx->screen, (SDL_Rect *)r);
		}
		SDL_UpdateWindowSurface(ctx->window);
	}
	return 1;
}

void win_fb_sdl_set_backlight(void * context, int brightness)
{
}

int win_fb_sdl_get_backlight(void * context)
{
	return 0;
}
