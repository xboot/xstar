#ifndef __WIN_H__
#define __WIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>

/*
 * Win region
 */
struct win_region_t {
	int x, y;
	int w, h;
};

static inline void win_region_init(struct win_region_t * r, int x, int y, int w, int h)
{
	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;
}

static inline void win_region_clone(struct win_region_t * r, struct win_region_t * o)
{
	r->x = o->x;
	r->y = o->y;
	r->w = o->w;
	r->h = o->h;
}

static inline int win_region_isempty(struct win_region_t * r)
{
	if((r->w > 0) && (r->h > 0))
		return 0;
	return 1;
}

static inline int win_region_hit(struct win_region_t * r, int x, int y)
{
	if((x >= r->x) && (x < r->x + r->w) && (y >= r->y) && (y < r->y + r->h))
		return 1;
	return 0;
}

static inline int win_region_contains(struct win_region_t * r, struct win_region_t * o)
{
	int rr = r->x + r->w;
	int rb = r->y + r->h;
	int or = o->x + o->w;
	int ob = o->y + o->h;
	if((o->x >= r->x) && (o->x < rr) && (o->y >= r->y) && (o->y < rb) && (or > r->x) && (or <= rr) && (ob > r->y) && (ob <= rb))
		return 1;
	return 0;
}

static inline int win_region_overlap(struct win_region_t * r, struct win_region_t * o)
{
	if((o->x + o->w >= r->x) && (o->x <= r->x + r->w) && (o->y + o->h >= r->y) && (o->y <= r->y + r->h))
		return 1;
	return 0;
}

static inline void win_region_expand(struct win_region_t * r, struct win_region_t * o, int n)
{
	r->x = o->x - n;
	r->y = o->y - n;
	r->w = o->w + n * 2;
	r->h = o->h + n * 2;
}

static inline int win_region_intersect(struct win_region_t * r, struct win_region_t * a, struct win_region_t * b)
{
	int x0 = XMAX(a->x, b->x);
	int x1 = XMIN(a->x + a->w, b->x + b->w);
	if(x0 <= x1)
	{
		int y0 = XMAX(a->y, b->y);
		int y1 = XMIN(a->y + a->h, b->y + b->h);
		if(y0 <= y1)
		{
			r->x = x0;
			r->y = y0;
			r->w = x1 - x0;
			r->h = y1 - y0;
			return 1;
		}
	}
	return 0;
}

static inline int win_region_union(struct win_region_t * r, struct win_region_t * a, struct win_region_t * b)
{
	int ar = a->x + a->w;
	int ab = a->y + a->h;
	int br = b->x + b->w;
	int bb = b->y + b->h;
	r->x = XMIN(a->x, b->x);
	r->y = XMIN(a->y, b->y);
	r->w = XMAX(ar, br) - r->x;
	r->h = XMAX(ab, bb) - r->y;
	return 1;
}

/*
 * Win interface
 */
void win_init(void);
void win_exit(void);

/*
 * Copyright interface
 */
char * win_copyright_uniqueid(void);
int win_copyright_verify(void);

/*
 * Win dirtylist
 */
struct win_dirtylist_item_t {
	struct win_region_t region;
	int area;
};

struct win_dirtylist_t {
	struct win_dirtylist_item_t * items;
	unsigned int size;
	unsigned int count;
};

struct win_dirtylist_t * win_dirtylist_alloc(unsigned int size);
void win_dirtylist_free(struct win_dirtylist_t * l);
void win_dirtylist_clone(struct win_dirtylist_t * l, struct win_dirtylist_t * o);
void win_dirtylist_merge(struct win_dirtylist_t * l, struct win_dirtylist_t * o);
void win_dirtylist_clear(struct win_dirtylist_t * l);
void win_dirtylist_add(struct win_dirtylist_t * l, struct win_region_t * r);

/*
 * Dma interface
 */
void * win_dma_alloc_coherent(unsigned long size);
void win_dma_free_coherent(void * addr);
void * win_dma_alloc_noncoherent(unsigned long size);
void win_dma_free_noncoherent(void * addr);
void win_dma_sync(void * addr, unsigned long size, int flag);

/* SDL event */
void win_event_sdl_mouse_set_range(int xmax, int ymax);
void win_event_sdl_mouse_get_range(int * xmax, int * ymax);
void win_event_sdl_mouse_set_sensitivity(int s);
void win_event_sdl_mouse_get_sensitivity(int * s);
void win_event_sdl_set_key_callback(void * device,
		void (*down)(void * device, unsigned int key),
		void (*up)(void * device, unsigned int key));
void win_event_sdl_set_mouse_callback(void * device,
		void (*down)(void * device, int x, int y, unsigned int button),
		void (*move)(void * device, int x, int y),
		void (*up)(void * device, int x, int y, unsigned int button),
		void (*wheel)(void * device, int dx, int dy));
void win_event_sdl_set_touch_callback(void * device,
		void (*begin)(void * device, int x, int y, unsigned int id),
		void (*move)(void * device, int x, int y, unsigned int id),
		void (*end)(void * device, int x, int y, unsigned int id));
void win_event_sdl_set_joystick_callback(void * device,
		void (*left_stick)(void * device, int x, int y),
		void (*right_stick)(void * device, int x, int y),
		void (*left_trigger)(void * device, int v),
		void (*right_trigger)(void * device, int v),
		void (*button_down)(void * device, unsigned int button),
		void (*button_up)(void * device, unsigned int button));
void win_event_sdl_poll(void);

/*
 * Framebuffer interface
 */
struct win_fb_surface_t {
	int width;
	int height;
	int stride;
	int pixlen;
	void * pixels;
	void * priv;
};

/* SDL windows */
void * win_fb_sdl_open(const char * title, int width, int height);
void win_fb_sdl_close(void * context);
int win_fb_sdl_get_width(void * context);
int win_fb_sdl_get_height(void * context);
int win_fb_sdl_get_pwidth(void * context);
int win_fb_sdl_get_pheight(void * context);
int win_fb_sdl_surface_create(void * context, struct win_fb_surface_t * surface, int width, int height);
int win_fb_sdl_surface_destroy(void * context, struct win_fb_surface_t * surface);
int win_fb_sdl_surface_present(void * context, struct win_fb_surface_t * surface, struct win_dirtylist_t * l);
void win_fb_sdl_set_backlight(void * context, int brightness);
int win_fb_sdl_get_backlight(void * context);

/*
 * File interface
 */
char * win_file_cwd(void);
int win_file_open(const char * path, const char * mode);
int win_file_close(int fd);
int win_file_isdir(const char * path);
int win_file_isfile(const char * path);
int win_file_mode(const char * path);
int win_file_mkdir(const char * path);
int win_file_remove(const char * path);
int win_file_access(const char * path, const char * mode);
void win_file_walk(const char * path, void (*cb)(const char * dir, const char * name, void * data), const char * dir, void * data);
ssize_t win_file_read(int fd, void * buf, size_t count);
ssize_t win_file_write(int fd, const void * buf, size_t count);
int64_t win_file_seek(int fd, int64_t offset);
int64_t win_file_tell(int fd);
int64_t win_file_length(int fd);
void win_file_sync(int fd);

/*
 * IO interface
 */
uint8_t win_io_read8(io_addr_t addr);
void win_io_write8(io_addr_t addr, uint8_t value);
uint16_t win_io_read16(io_addr_t addr);
void win_io_write16(io_addr_t addr, uint16_t value);
uint32_t win_io_read32(io_addr_t addr);
void win_io_write32(io_addr_t addr, uint32_t value);
uint64_t win_io_read64(io_addr_t addr);
void win_io_write64(io_addr_t addr, uint64_t value);

/*
 * Memory interface
 */
void * win_mem_malloc(size_t size);
void * win_mem_memalign(size_t align, size_t size);
void * win_mem_realloc(void * ptr, size_t size);
void * win_mem_calloc(size_t nmemb, size_t size);
void win_mem_free(void * ptr);
void win_mem_meminfo(size_t * mused, size_t * mfree);

/*
 * Mutex interface
 */
void win_mutex_init(struct mutex_t * lock);
void win_mutex_exit(struct mutex_t * lock);
int win_mutex_lock(struct mutex_t * lock);
int win_mutex_trylock(struct mutex_t * lock);
int win_mutex_unlock(struct mutex_t * lock);

/*
 * PM interface
 */
void win_pm_shutdown(void);
void win_pm_reboot(void);
void win_pm_standby(void);

/*
 * Semaphore interface
 */
void win_semaphore_init(struct semaphore_t * sem, uint32_t count);
void win_semaphore_exit(struct semaphore_t * sem);
int win_semaphore_wait(struct semaphore_t * sem, uint32_t timeout);
int win_semaphore_post(struct semaphore_t * sem);

/*
 * Stdio interface
 */
ssize_t win_stdio_read(void * buf, size_t count);
ssize_t win_stdio_write(void * buf, size_t count);

/*
 * Thread interface
 */
struct thread_t * win_thread_create(const char * name, void (*func)(void *), void * data, int stksz);
void win_thread_destroy(struct thread_t * thread);
void win_thread_wait(struct thread_t * thread);
void win_thread_sleep(uint64_t ns);

/*
 * Timer interface
 */
void win_timer_init(void);
void win_timer_exit(void);
void win_timer_next(uint64_t time, void (*cb)(void *), void * data);
uint64_t win_timer_count(void);
uint64_t win_timer_frequency(void);
uint64_t win_realtime(void);

/*
 * Coroutine interface
 */
void * x64_coroutine_make(void * stack, size_t size, void (*func)(struct co_transfer_t));
struct co_transfer_t x64_coroutine_jump(void * fctx, void * priv);

#ifdef __cplusplus
}
#endif

#endif /* __WIN_H__ */
