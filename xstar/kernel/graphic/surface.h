#ifndef __XSTAR_KERNEL_GRAPHIC_SURFACE_H__
#define __XSTAR_KERNEL_GRAPHIC_SURFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/graphic/region.h>
#include <kernel/graphic/dirtylist.h>
#include <kernel/graphic/color.h>
#include <kernel/graphic/matrix2d.h>
#include <kernel/graphic/blur.h>
#include <kernel/graphic/effect.h>
#include <kernel/graphic/filter.h>
#include <kernel/font/font.h>
#include <kernel/xfs/xfs.h>
#include <driver/g2d/g2d.h>
#include <kernel/cg/cg.h>

struct cg_render_t {
	struct cg_surface_t * surface;
	struct cg_ctx_t * ctx;
};

/*
 * Each pixel is a 32-bits, with alpha in the upper 8 bits, then red green and blue.
 * The 32-bit quantities are stored native-endian, Pre-multiplied alpha is used.
 * That is, 50% transparent red is 0x80800000 not 0x80ff0000.
 */
struct surface_t {
	int width;
	int height;
	int stride;
	int pixlen;
	void * pixels;
	struct g2d_t * g2d;
	struct cg_render_t * cg;
	void * priv;
};

static inline int surface_get_width(struct surface_t * s)
{
	return s->width;
}

static inline int surface_get_height(struct surface_t * s)
{
	return s->height;
}

static inline int surface_get_stride(struct surface_t * s)
{
	return s->stride;
}

static inline void * surface_get_pixels(struct surface_t * s)
{
	return s->pixels;
}

static inline int surface_get_pixlen(struct surface_t * s)
{
	return s->pixlen;
}

struct cg_surface_t * surface_get_cg_surface(struct surface_t * s);
struct cg_ctx_t * surface_get_cg_ctx(struct surface_t * s);

struct surface_t * surface_alloc(int width, int height);
struct surface_t * surface_alloc_from_xfs(struct xfs_context_t * ctx, const char * filename);
struct surface_t * surface_alloc_from_buf(const void * buf, int len);
struct surface_t * surface_alloc_qrcode(int pixsz, const char * fmt, ...);
void surface_free(struct surface_t * s);

struct surface_t * surface_clone(struct surface_t * s, int x, int y, int w, int h);
struct surface_t * surface_extend(struct surface_t * s, int width, int height, const char * type);
void surface_clear(struct surface_t * s, struct color_t * c, int x, int y, int w, int h);
void surface_set_pixel(struct surface_t * s, int x, int y, struct color_t * c);
void surface_get_pixel(struct surface_t * s, int x, int y, struct color_t * c);
void surface_text(struct surface_t * s, struct region_t * clip, int x, int y, int wrap, const char * family, enum font_style_t style, int size, struct color_t * c, const char * fmt, ...);
void surface_icon(struct surface_t * s, struct region_t * clip, int x, int y, const char * family, int size, uint32_t code, struct color_t * c);

void surface_blit(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, struct surface_t * o);
void surface_fill(struct surface_t * s, struct region_t * clip, struct matrix2d_t * m, int w, int h, struct color_t * c);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_GRAPHIC_SURFACE_H__ */
