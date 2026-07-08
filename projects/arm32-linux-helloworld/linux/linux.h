#ifndef __LINUX_H__
#define __LINUX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>

/*
 * Linux region
 */
struct linux_region_t {
	int x, y;
	int w, h;
};

static inline void linux_region_init(struct linux_region_t * r, int x, int y, int w, int h)
{
	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;
}

static inline void linux_region_clone(struct linux_region_t * r, struct linux_region_t * o)
{
	r->x = o->x;
	r->y = o->y;
	r->w = o->w;
	r->h = o->h;
}

static inline int linux_region_isempty(struct linux_region_t * r)
{
	if((r->w > 0) && (r->h > 0))
		return 0;
	return 1;
}

static inline int linux_region_hit(struct linux_region_t * r, int x, int y)
{
	if((x >= r->x) && (x < r->x + r->w) && (y >= r->y) && (y < r->y + r->h))
		return 1;
	return 0;
}

static inline int linux_region_contains(struct linux_region_t * r, struct linux_region_t * o)
{
	int rr = r->x + r->w;
	int rb = r->y + r->h;
	int or = o->x + o->w;
	int ob = o->y + o->h;
	if((o->x >= r->x) && (o->x < rr) && (o->y >= r->y) && (o->y < rb) && (or > r->x) && (or <= rr) && (ob > r->y) && (ob <= rb))
		return 1;
	return 0;
}

static inline int linux_region_overlap(struct linux_region_t * r, struct linux_region_t * o)
{
	if((o->x + o->w >= r->x) && (o->x <= r->x + r->w) && (o->y + o->h >= r->y) && (o->y <= r->y + r->h))
		return 1;
	return 0;
}

static inline void linux_region_expand(struct linux_region_t * r, struct linux_region_t * o, int n)
{
	r->x = o->x - n;
	r->y = o->y - n;
	r->w = o->w + n * 2;
	r->h = o->h + n * 2;
}

static inline int linux_region_intersect(struct linux_region_t * r, struct linux_region_t * a, struct linux_region_t * b)
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

static inline int linux_region_union(struct linux_region_t * r, struct linux_region_t * a, struct linux_region_t * b)
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
 * Linux interface
 */
void linux_init(void);
void linux_exit(void);

/*
 * Audio playback interface
 */
void * linux_audio_playback_start(int rate, int channel);
int linux_audio_playback_write(void * context, float * samples, int nsample);
void linux_audio_playback_stop(void * context);
void linux_audio_playback_set_volume(int vol);
int linux_audio_playback_get_volume(void);

/*
 * Audio capture interface
 */
void * linux_audio_capture_start(int rate, int channel);
int linux_audio_capture_read(void * context, float * samples, int nsample);
void linux_audio_capture_stop(void * context);
void linux_audio_capture_set_volume(int vol);
int linux_audio_capture_get_volume(void);

/*
 * Camera interface
 */
int linux_cam_exist(const char * dev);
void * linux_cam_start(const char * dev, int * format, int * width, int * height);
void linux_cam_stop(void * context);
int linux_cam_capture(void * context, void ** buf);

/*
 * Copyright interface
 */
char * linux_copyright_uniqueid(void);
int linux_copyright_verify(void);

/*
 * Linux dirtylist
 */
struct linux_dirtylist_item_t {
	struct linux_region_t region;
	int area;
};

struct linux_dirtylist_t {
	struct linux_dirtylist_item_t * items;
	unsigned int size;
	unsigned int count;
};

struct linux_dirtylist_t * linux_dirtylist_alloc(unsigned int size);
void linux_dirtylist_free(struct linux_dirtylist_t * l);
void linux_dirtylist_clone(struct linux_dirtylist_t * l, struct linux_dirtylist_t * o);
void linux_dirtylist_merge(struct linux_dirtylist_t * l, struct linux_dirtylist_t * o);
void linux_dirtylist_clear(struct linux_dirtylist_t * l);
void linux_dirtylist_add(struct linux_dirtylist_t * l, struct linux_region_t * r);

/*
 * Dma interface
 */
void * linux_dma_alloc_coherent(unsigned long size);
void linux_dma_free_coherent(void * addr);
void * linux_dma_alloc_noncoherent(unsigned long size);
void linux_dma_free_noncoherent(void * addr);
void linux_dma_sync(void * addr, unsigned long size, int flag);

/*
 * Event interface
 */
/* Input device */
void * linux_event_open(void);
void linux_event_close(void * context);
void linux_event_mouse_set_range(void * context, int xmax, int ymax);
void linux_event_mouse_get_range(void * context, int * xmax, int * ymax);
void linux_event_mouse_set_sensitivity(void * context, int s);
void linux_event_mouse_get_sensitivity(void * context, int * s);
void linux_event_set_key_callback(void * context, void * device,
		void (*down)(void * device, unsigned int key),
		void (*up)(void * device, unsigned int key));
void linux_event_set_mouse_callback(void * context, void * device,
		void (*down)(void * device, int x, int y, unsigned int button),
		void (*move)(void * device, int x, int y),
		void (*up)(void * device, int x, int y, unsigned int button),
		void (*wheel)(void * device, int dx, int dy));
void linux_event_set_touch_callback(void * context, void * device,
		void (*begin)(void * device, int x, int y, unsigned int id),
		void (*move)(void * device, int x, int y, unsigned int id),
		void (*end)(void * device, int x, int y, unsigned int id));
void linux_event_set_joystick_callback(void * context, void * device,
		void (*left_stick)(void * device, int x, int y),
		void (*right_stick)(void * device, int x, int y),
		void (*left_trigger)(void * device, int v),
		void (*right_trigger)(void * device, int v),
		void (*button_down)(void * device, unsigned int button),
		void (*button_up)(void * device, unsigned int button));

/* SDL event */
void * linux_event_sdl_open(void);
void linux_event_sdl_close(void * context);
void linux_event_sdl_mouse_set_range(void * context, int xmax, int ymax);
void linux_event_sdl_mouse_get_range(void * context, int * xmax, int * ymax);
void linux_event_sdl_mouse_set_sensitivity(void * context, int s);
void linux_event_sdl_mouse_get_sensitivity(void * context, int * s);
void linux_event_sdl_set_key_callback(void * context, void * device,
		void (*down)(void * device, unsigned int key),
		void (*up)(void * device, unsigned int key));
void linux_event_sdl_set_mouse_callback(void * context, void * device,
		void (*down)(void * device, int x, int y, unsigned int button),
		void (*move)(void * device, int x, int y),
		void (*up)(void * device, int x, int y, unsigned int button),
		void (*wheel)(void * device, int dx, int dy));
void linux_event_sdl_set_touch_callback(void * context, void * device,
		void (*begin)(void * device, int x, int y, unsigned int id),
		void (*move)(void * device, int x, int y, unsigned int id),
		void (*end)(void * device, int x, int y, unsigned int id));
void linux_event_sdl_set_joystick_callback(void * context, void * device,
		void (*left_stick)(void * device, int x, int y),
		void (*right_stick)(void * device, int x, int y),
		void (*left_trigger)(void * device, int v),
		void (*right_trigger)(void * device, int v),
		void (*button_down)(void * device, unsigned int button),
		void (*button_up)(void * device, unsigned int button));

/*
 * Framebuffer interface
 */
struct linux_fb_surface_t {
	int width;
	int height;
	int stride;
	int pixlen;
	void * pixels;
	void * priv;
};

/* Framebuffer device */
void * linux_fb_open(const char * dev);
void linux_fb_close(void * context);
int linux_fb_get_width(void * context);
int linux_fb_get_height(void * context);
int linux_fb_get_pwidth(void * context);
int linux_fb_get_pheight(void * context);
int linux_fb_surface_create(void * context, struct linux_fb_surface_t * surface, int width, int height);
int linux_fb_surface_destroy(void * context, struct linux_fb_surface_t * surface);
int linux_fb_surface_present(void * context, struct linux_fb_surface_t * surface, struct linux_dirtylist_t * l);
void linux_fb_set_backlight(void * context, int brightness);
int linux_fb_get_backlight(void * context);

/* DRM device */
void * linux_fb_drm_open(const char * dev, const char * connector);
void linux_fb_drm_close(void * context);
int linux_fb_drm_get_width(void * context);
int linux_fb_drm_get_height(void * context);
int linux_fb_drm_get_pwidth(void * context);
int linux_fb_drm_get_pheight(void * context);
int linux_fb_drm_surface_create(void * context, struct linux_fb_surface_t * surface, int width, int height);
int linux_fb_drm_surface_destroy(void * context, struct linux_fb_surface_t * surface);
int linux_fb_drm_surface_present(void * context, struct linux_fb_surface_t * surface, struct linux_dirtylist_t * l);
void linux_fb_drm_set_backlight(void * context, int brightness);
int linux_fb_drm_get_backlight(void * context);

/* SDL windows */
void * linux_fb_sdl_open(const char * title, int width, int height);
void linux_fb_sdl_close(void * context);
int linux_fb_sdl_get_width(void * context);
int linux_fb_sdl_get_height(void * context);
int linux_fb_sdl_get_pwidth(void * context);
int linux_fb_sdl_get_pheight(void * context);
int linux_fb_sdl_surface_create(void * context, struct linux_fb_surface_t * surface, int width, int height);
int linux_fb_sdl_surface_destroy(void * context, struct linux_fb_surface_t * surface);
int linux_fb_sdl_surface_present(void * context, struct linux_fb_surface_t * surface, struct linux_dirtylist_t * l);
void linux_fb_sdl_set_backlight(void * context, int brightness);
int linux_fb_sdl_get_backlight(void * context);

/*
 * File interface
 */
char * linux_file_cwd(void);
int linux_file_open(const char * path, const char * mode);
int linux_file_close(int fd);
int linux_file_isdir(const char * path);
int linux_file_isfile(const char * path);
int linux_file_mode(const char * path);
int linux_file_mkdir(const char * path);
int linux_file_remove(const char * path);
int linux_file_access(const char * path, const char * mode);
void linux_file_walk(const char * path, void (*cb)(const char * dir, const char * name, void * data), const char * dir, void * data);
ssize_t linux_file_read(int fd, void * buf, size_t count);
ssize_t linux_file_read_nonblock(int fd, void * buf, size_t count);
ssize_t linux_file_write(int fd, const void * buf, size_t count);
int64_t linux_file_seek(int fd, int64_t offset);
int64_t linux_file_tell(int fd);
int64_t linux_file_length(int fd);
void linux_file_sync(int fd);

/*
 * I2C interface
 */
struct linux_i2c_msg_t {
	int addr;
	int flags;
	int len;
	void * buf;
};
int linux_i2c_exist(const char * dev);
int linux_i2c_open(const char * dev);
void linux_i2c_close(int fd);
int linux_i2c_xfer(int fd, struct linux_i2c_msg_t * msgs, int num);

/*
 * IO interface
 */
uint8_t linux_io_read8(io_addr_t addr);
void linux_io_write8(io_addr_t addr, uint8_t value);
uint16_t linux_io_read16(io_addr_t addr);
void linux_io_write16(io_addr_t addr, uint16_t value);
uint32_t linux_io_read32(io_addr_t addr);
void linux_io_write32(io_addr_t addr, uint32_t value);
uint64_t linux_io_read64(io_addr_t addr);
void linux_io_write64(io_addr_t addr, uint64_t value);

/*
 * Memory interface
 */
void * linux_mem_malloc(size_t size);
void * linux_mem_memalign(size_t align, size_t size);
void * linux_mem_realloc(void * ptr, size_t size);
void * linux_mem_calloc(size_t nmemb, size_t size);
void linux_mem_free(void * ptr);
void linux_mem_meminfo(size_t * mused, size_t * mfree);

/*
 * Mutex interface
 */
void linux_mutex_init(struct mutex_t * lock);
void linux_mutex_exit(struct mutex_t * lock);
int linux_mutex_lock(struct mutex_t * lock);
int linux_mutex_trylock(struct mutex_t * lock);
int linux_mutex_unlock(struct mutex_t * lock);

/*
 * PM interface
 */
void linux_pm_shutdown(void);
void linux_pm_reboot(void);
void linux_pm_standby(void);

/*
 * RTC interface
 */
struct linux_rtc_time_t {
	unsigned char second;	/* second [0 - 59] */
	unsigned char minute;	/* minute [0 - 59] */
	unsigned char hour;		/* hour [0 - 23] */
	unsigned char week;		/* week [0 - 6] */
	unsigned char day;		/* day [1 - 31] */
	unsigned char month;	/* month [1 - 12] */
	unsigned int  year;		/* year */
};

int linux_rtc_exist(const char * dev);
int linux_rtc_settime(const char * dev, struct linux_rtc_time_t * time);
int linux_rtc_gettime(const char * dev, struct linux_rtc_time_t * time);

/*
 * Semaphore interface
 */
void linux_semaphore_init(struct semaphore_t * sem, uint32_t count);
void linux_semaphore_exit(struct semaphore_t * sem);
int linux_semaphore_wait(struct semaphore_t * sem, uint32_t timeout);
int linux_semaphore_post(struct semaphore_t * sem);

/*
 * Shell interface
 */
void linux_shell(const char * cmd, char * msg, int sz, int async);

/*
 * Socket interface
 */
void * linux_socket_listen(const char * type, int port);
void * linux_socket_accept(void * l);
void * linux_socket_connect(const char * type, const char * host, int port);
int linux_socket_read(void * c, void * buf, int count);
int linux_socket_write(void * c, void * buf, int count);
int linux_socket_status(void * c);
void linux_socket_close(void * c);
void linux_socket_delete(void * l);
int linux_socket_get_ip(const char * iface, char * ip);
int linux_socket_get_mac(const char * iface, char * mac);

/*
 * SPI interface
 */
struct linux_spi_msg_t {
	void * txbuf;
	void * rxbuf;
	int len;
	int type;
	int bits;
	int speed;
};
int linux_spi_exist(const char * dev);
int linux_spi_open(const char * dev);
void linux_spi_close(int fd);
void linux_spi_mode(int fd, int mode);
int linux_spi_transfer(int fd, struct linux_spi_msg_t * msg);
void linux_spi_select(int fd, int cs);
void linux_spi_deselect(int fd, int cs);

/*
 * Stdio interface
 */
ssize_t linux_stdio_read(void * buf, size_t count);
ssize_t linux_stdio_write(void * buf, size_t count);

/*
 * Sysfs interface
 */
int linux_sysfs_access(const char * path, const char * mode);
int linux_sysfs_read_value(const char * path, int * v);
int linux_sysfs_write_value(const char * path, const int v);
int linux_sysfs_read_string(const char * path, char * s);
int linux_sysfs_write_string(const char * path, const char * s);

/*
 * Thread interface
 */
struct thread_t * linux_thread_create(const char * name, void (*func)(void *), void * data, int stksz);
void linux_thread_destroy(struct thread_t * thread);
void linux_thread_wait(struct thread_t * thread);
void linux_thread_sleep(uint64_t ns);

/*
 * Timer interface
 */
void linux_timer_init(void);
void linux_timer_exit(void);
void linux_timer_next(uint64_t time, void (*cb)(void *), void * data);
uint64_t linux_timer_count(void);
uint64_t linux_timer_frequency(void);
uint64_t linux_realtime(void);

/*
 * Uart interface
 */
int linux_uart_open(const char * dev);
int linux_uart_close(int fd);
int linux_uart_set(int fd, int baud, int data, int parity, int stop);
ssize_t linux_uart_read(int fd, void * buf, size_t len);
ssize_t linux_uart_write(int fd, const void * buf, size_t len);

/*
 * Coroutine interface
 */
void * arm32_coroutine_make(void * stack, size_t size, void (*func)(struct co_transfer_t));
struct co_transfer_t arm32_coroutine_jump(void * fctx, void * priv);

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_H__ */
