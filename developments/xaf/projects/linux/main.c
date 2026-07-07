#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <xaf.h>

struct player_t {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	struct xaf_t *ctx;
	uint8_t *data;
	int width;
	int height;
	int nframes;
	int fps;
	int current_frame;
	int paused;
	int need_decode;
	uint32_t frame_delay;
	uint32_t last_tick;
};

static void player_cleanup(struct player_t *p)
{
	if(p->texture)
	{
		SDL_DestroyTexture(p->texture);
		p->texture = NULL;
	}
	if(p->ctx)
	{
		xaf_free(p->ctx);
		p->ctx = NULL;
	}
	if(p->data)
	{
		free(p->data);
		p->data = NULL;
	}
}

static void player_reload(struct player_t *p, const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if(!fp)
	{
		fprintf(stderr, "Error: cannot open %s\n", filename);
		return;
	}
	fseek(fp, 0, SEEK_END);
	int file_size = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	uint8_t *new_data = (uint8_t*)malloc(file_size);
	if((int)fread(new_data, 1, file_size, fp) != file_size)
	{
		fprintf(stderr, "Error: failed to read file\n");
		free(new_data);
		fclose(fp);
		return;
	}
	fclose(fp);

	struct xaf_t *new_ctx = xaf_alloc(new_data, file_size);
	if(!new_ctx)
	{
		fprintf(stderr, "Error: invalid XAF file\n");
		free(new_data);
		return;
	}

	player_cleanup(p);
	p->data = new_data;
	p->ctx = new_ctx;

	p->width = xaf_get_width(p->ctx);
	p->height = xaf_get_height(p->ctx);
	p->nframes = xaf_get_nframes(p->ctx);
	p->fps = xaf_get_fps(p->ctx);

	printf("XAF: %dx%d, %d frames, %d fps (Cinepak)\n", p->width, p->height, p->nframes, p->fps);

	if(p->renderer)
		SDL_DestroyRenderer(p->renderer);

	SDL_SetWindowSize(p->window, p->width, p->height);
	p->renderer = SDL_CreateRenderer(p->window, -1, SDL_RENDERER_SOFTWARE);
	if(!p->renderer)
	{
		fprintf(stderr, "Error: create renderer: %s\n", SDL_GetError());
		player_cleanup(p);
		return;
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	SDL_RenderSetLogicalSize(p->renderer, p->width, p->height);

	p->texture = SDL_CreateTexture(p->renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, p->width, p->height);
	if(!p->texture)
	{
		fprintf(stderr, "Error: create texture: %s\n", SDL_GetError());
		return;
	}

	p->current_frame = 0;
	p->need_decode = 1;
	p->paused = 0;
	p->frame_delay = p->fps > 0 ? 1000 / p->fps : 40;
	p->last_tick = SDL_GetTicks();

	char title[256];
	snprintf(title, sizeof(title), "XAF - %s", filename);
	SDL_SetWindowTitle(p->window, title);
}

static void player_init(struct player_t *p)
{
	memset(p, 0, sizeof(*p));
	p->width = 640;
	p->height = 480;
}

static void player_destroy(struct player_t *p)
{
	player_cleanup(p);
	if(p->renderer)
		SDL_DestroyRenderer(p->renderer);
	if(p->window)
		SDL_DestroyWindow(p->window);
}

int main(int argc, char *argv[])
{
	struct player_t p;

	player_init(&p);

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "Error: SDL init: %s\n", SDL_GetError());
		return 1;
	}

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	p.window = SDL_CreateWindow("XAF - Drop file to play", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, p.width, p.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	if(!p.window)
	{
		fprintf(stderr, "Error: create window: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	p.renderer = SDL_CreateRenderer(p.window, -1, SDL_RENDERER_SOFTWARE);
	if(!p.renderer)
	{
		fprintf(stderr, "Error: create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(p.window);
		SDL_Quit();
		return 1;
	}

	printf("Drag & drop an XAF file to play. (Space=pause, R=restart, Q/Esc=quit)\n");

	if(argc >= 2)
		player_reload(&p, argv[1]);

	p.last_tick = SDL_GetTicks();
	int running = 1;

	while(running)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT)
				running = 0;
			if(e.type == SDL_DROPFILE)
			{
				player_reload(&p, e.drop.file);
				SDL_free(e.drop.file);
			}
			if(e.type == SDL_KEYDOWN)
			{
				switch(e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
				case SDLK_q:
					running = 0;
					break;
				case SDLK_SPACE:
					p.paused = !p.paused;
					printf("%s at frame %d/%d\n", p.paused ? "Paused" : "Playing", p.current_frame + 1, p.nframes);
					break;
				case SDLK_r:
					if(p.ctx)
					{
						xaf_reset(p.ctx);
						p.current_frame = 0;
						p.need_decode = 1;
						p.last_tick = SDL_GetTicks();
					}
					break;
				}
			}
		}

		if(p.ctx)
		{
			uint32_t now = SDL_GetTicks();
			if(!p.paused && now - p.last_tick >= p.frame_delay)
			{
				if(p.current_frame + 1 >= p.nframes)
				{
					xaf_reset(p.ctx);
					p.current_frame = 0;
				}
				else
				{
					p.current_frame++;
				}
				p.need_decode = 1;
				p.last_tick += p.frame_delay;
			}

			if(p.need_decode)
			{
				if(!xaf_next(p.ctx))
				{
					fprintf(stderr, "Error: decode frame %d failed\n", p.current_frame);
					break;
				}
				SDL_UpdateTexture(p.texture, NULL, p.ctx->frame, p.width * 4);
				SDL_RenderClear(p.renderer);
				SDL_RenderCopy(p.renderer, p.texture, NULL, NULL);
				SDL_RenderPresent(p.renderer);

				char t[512];
				snprintf(t, sizeof(t), "XAF [%d/%d]%s", p.current_frame + 1, p.nframes, p.paused ? " PAUSED" : "");
				SDL_SetWindowTitle(p.window, t);

				p.need_decode = 0;
			}
		}
		else
		{
			SDL_RenderClear(p.renderer);
			SDL_RenderPresent(p.renderer);
		}

		SDL_Delay(1);
	}

	player_destroy(&p);
	SDL_Quit();
	return 0;
}
