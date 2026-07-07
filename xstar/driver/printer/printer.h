#ifndef __XSTAR_DRIVER_PRINTER_H__
#define __XSTAR_DRIVER_PRINTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <kernel/vision/vision.h>
#include <kernel/vision/dither.h>
#include <kernel/core/thworker.h>

enum printer_align_t {
	PRINTER_ALIGN_LEFT				= (0x1 << 0),
	PRINTER_ALIGN_CENTER			= (0x2 << 0),
	PRINTER_ALIGN_RIGHT				= (0x3 << 0),
};
#define PRINTER_ALIGN_OFFSET(x)		((enum printer_align_t)((int)(x) << 2))

struct printer_t {
	char * name;

	void (*print)(struct printer_t * p, unsigned char * buf, unsigned int len);
	void (*feed)(struct printer_t * p, int pixel);
	void (*cut)(struct printer_t * p);
	void (*standby)(struct printer_t * p);
	int (*ioctl)(struct printer_t * p, const char * cmd, void * arg);
	void * priv;
};

static inline int printer_ioctl(struct printer_t * p, const char * cmd, void * arg)
{
	if(p && p->ioctl)
		return p->ioctl(p, cmd, arg);
	return -1;
}

static inline int printer_get_dpl(struct printer_t * p)
{
	int dpl;

	if(printer_ioctl(p, "printer-get-dpl", &dpl) >= 0)
		return dpl;
	return 0;
}

static inline int printer_get_dpi(struct printer_t * p)
{
	int dpi;

	if(printer_ioctl(p, "printer-get-dpi", &dpi) >= 0)
		return dpi;
	return 0;
}

static inline int printer_get_paper(struct printer_t * p)
{
	int paper;

	if(printer_ioctl(p, "printer-get-paper", &paper) >= 0)
		return paper;
	return 0;
}

static inline int printer_get_temperature(struct printer_t * p)
{
	int temp;

	if(printer_ioctl(p, "printer-get-temperature", &temp) >= 0)
		return temp;
	return 0;
}

static inline float printer_get_density(struct printer_t * p)
{
	float density;

	if(printer_ioctl(p, "printer-get-density", &density) >= 0)
		return density;
	return 0.5f;
}

static inline int printer_set_density(struct printer_t * p, float density)
{
	density = XCLAMP(density, 0.0f, 1.0f);
	if(printer_ioctl(p, "printer-set-density", &density) >= 0)
		return 1;
	return 0;
}

struct printer_t * search_printer(const char * name);
struct printer_t * search_first_printer(void);
struct device_t * register_printer(struct printer_t * p, struct driver_t * drv);
void unregister_printer(struct printer_t * p);

void printer_print(struct printer_t * p, unsigned char * buf, unsigned int len);
void printer_feed(struct printer_t * p, int lines);
void printer_cut(struct printer_t * p);
void printer_standby(struct printer_t * p);

void printer_print_hr(struct printer_t * p, int black, int white);
void printer_print_text(struct printer_t * p, enum printer_align_t align, const char * family, enum font_style_t style, int size, const char * fmt, ...);
void printer_print_qrcode(struct printer_t * p, enum printer_align_t align, int pixsz, const char * fmt, ...);
void printer_print_vision(struct printer_t * p, enum printer_align_t align, struct vision_t * v);
void printer_print_surface(struct printer_t * p, enum printer_align_t align, struct surface_t * s);

/*
 * printer async
 */
struct printer_async_ctx_t {
	struct printer_t * printer;
	struct thworker_t * worker;
};

struct printer_async_ctx_t * printer_async_ctx_alloc(const char * name);
void printer_async_ctx_free(struct printer_async_ctx_t * ctx);
void printer_async_ctx_clear(struct printer_async_ctx_t * ctx);

void printer_async_print(struct printer_async_ctx_t * ctx, unsigned char * buf, unsigned int len);
void printer_async_feed(struct printer_async_ctx_t * ctx, int lines);
void printer_async_cut(struct printer_async_ctx_t * ctx);
void printer_async_standby(struct printer_async_ctx_t * ctx);

void printer_async_print_hr(struct printer_async_ctx_t * ctx, int black, int white);
void printer_async_print_text(struct printer_async_ctx_t * ctx, enum printer_align_t align, const char * family, enum font_style_t style, int size, const char * fmt, ...);
void printer_async_print_qrcode(struct printer_async_ctx_t * ctx, enum printer_align_t align, int pixsz, const char * fmt, ...);
void printer_async_print_vision(struct printer_async_ctx_t * ctx, enum printer_align_t align, struct vision_t * v);
void printer_async_print_surface(struct printer_async_ctx_t * ctx, enum printer_align_t align, struct surface_t * s);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_PRINTER_H__ */
