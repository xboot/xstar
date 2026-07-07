#include <linux/linux.h>

struct linux_fb_sdl_context_t {
	SDL_Window * window;
	SDL_Surface * screen;
	int width, height;
	int pwidth, pheight;
};

void * linux_fb_sdl_open(const char * title, int width, int height)
{
	struct linux_fb_sdl_context_t * ctx;
	Uint32 flags = SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_ALWAYS_ON_TOP;

	ctx = malloc(sizeof(struct linux_fb_sdl_context_t));
	if(!ctx)
		return NULL;

	ctx->window = SDL_CreateWindow(title ? title : "Xstar Runtime Environment", width, height, flags);
	if(!ctx->window)
	{
		free(ctx);
		return NULL;
	}
	SDL_SetWindowPosition(ctx->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	ctx->screen = SDL_GetWindowSurface(ctx->window);
	if(!ctx->screen)
	{
		if(ctx->window)
			SDL_DestroyWindow(ctx->window);
		free(ctx);
		return NULL;
	}

	float dpi = SDL_GetWindowPixelDensity(ctx->window);
	SDL_GetWindowSizeInPixels(ctx->window, &ctx->width, &ctx->height);
	ctx->pwidth = (int)(25.4 * ctx->width / dpi);
	ctx->pheight = (int)(25.4 * ctx->height / dpi);

	return ctx;
}

void linux_fb_sdl_close(void * context)
{
	struct linux_fb_sdl_context_t * ctx = (struct linux_fb_sdl_context_t *)context;

	if(!ctx)
		return;
	if(ctx->window)
		SDL_DestroyWindow(ctx->window);
	if(ctx->screen)
		SDL_DestroySurface(ctx->screen);
	free(ctx);
}

int linux_fb_sdl_get_width(void * context)
{
	struct linux_fb_sdl_context_t * ctx = (struct linux_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->width;
	return 0;
}

int linux_fb_sdl_get_height(void * context)
{
	struct linux_fb_sdl_context_t * ctx = (struct linux_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->height;
	return 0;
}

int linux_fb_sdl_get_pwidth(void * context)
{
	struct linux_fb_sdl_context_t * ctx = (struct linux_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->pwidth;
	return 0;
}

int linux_fb_sdl_get_pheight(void * context)
{
	struct linux_fb_sdl_context_t * ctx = (struct linux_fb_sdl_context_t *)context;
	if(ctx)
		return ctx->pheight;
	return 0;
}

int linux_fb_sdl_surface_create(void * context, struct linux_fb_surface_t * surface, int width, int height)
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

int linux_fb_sdl_surface_destroy(void * context, struct linux_fb_surface_t * surface)
{
	if(surface)
		SDL_DestroySurface(surface->priv);
	return 1;
}

int linux_fb_sdl_surface_present(void * context, struct linux_fb_surface_t * surface, struct linux_dirtylist_t * l)
{
	struct linux_fb_sdl_context_t * ctx = (struct linux_fb_sdl_context_t *)context;
	struct linux_region_t * r;
	int i;

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

void linux_fb_sdl_set_backlight(void * context, int brightness)
{
}

int linux_fb_sdl_get_backlight(void * context)
{
	return 0;
}
