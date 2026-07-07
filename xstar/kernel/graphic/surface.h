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
#include <kernel/font/font.h>
#include <kernel/xfs/xfs.h>

struct surface_t;
struct render_t;

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
	struct render_t * r;
	void * rctx;
	void * g2d;
	void * priv;
};

struct render_t {
	char * name;
	struct list_head_t list;

	void * (*create)(struct surface_t * s);
	void (*destroy)(void * rctx);

	void (*shape_save)(struct surface_t * s);
	void (*shape_restore)(struct surface_t * s);
	void (*shape_set_source)(struct surface_t * s, struct surface_t * o, float x, float y);
	void (*shape_set_source_color)(struct surface_t * s, struct color_t * c);
	void (*shape_set_fill_rule)(struct surface_t * s, const char * t);
	void (*shape_set_line_width)(struct surface_t * s, float w);
	void (*shape_set_line_cap)(struct surface_t * s, const char * t);
	void (*shape_set_line_join)(struct surface_t * s, const char * t);
	void (*shape_set_miter_limit)(struct surface_t * s, float l);
	void (*shape_set_dash)(struct surface_t * s, float * dashes, int ndash, float offset);
	void (*shape_translate)(struct surface_t * s, float tx, float ty);
	void (*shape_scale)(struct surface_t * s, float sx, float sy);
	void (*shape_rotate)(struct surface_t * s, float r);
	void (*shape_transform)(struct surface_t * s, struct matrix2d_t * m);
	void (*shape_set_matrix)(struct surface_t * s, struct matrix2d_t * m);
	void (*shape_identity_matrix)(struct surface_t * s);
	void (*shape_new_path)(struct surface_t * s);
	void (*shape_close_path)(struct surface_t * s);
	void (*shape_move_to)(struct surface_t * s, float x, float y);
	void (*shape_line_to)(struct surface_t * s, float x, float y);
	void (*shape_curve_to)(struct surface_t * s, float x1, float y1, float x2, float y2, float x3, float y3);
	void (*shape_rectangle)(struct surface_t * s, float x, float y, float w, float h);
	void (*shape_round_rectangle)(struct surface_t * s, float x, float y, float w, float h, float r);
	void (*shape_arc)(struct surface_t * s, float cx, float cy, float r, float a0, float a1);
	void (*shape_arc_negative)(struct surface_t * s, float cx, float cy, float r, float a0, float a1);
	void (*shape_circle)(struct surface_t * s, float cx, float cy, float r);
	void (*shape_ellipse)(struct surface_t * s, float cx, float cy, float rx, float ry);
	void (*shape_clip)(struct surface_t * s);
	void (*shape_clip_preserve)(struct surface_t * s);
	void (*shape_fill)(struct surface_t * s);
	void (*shape_fill_preserve)(struct surface_t * s);
	void (*shape_stroke)(struct surface_t * s);
	void (*shape_stroke_preserve)(struct surface_t * s);
	void (*shape_paint)(struct surface_t * s);
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

static inline void surface_set_priv(struct surface_t * s, void * priv)
{
	s->priv = priv;
}

static inline void * surface_get_priv(struct surface_t * s)
{
	return s->priv;
}

static inline void surface_shape_save(struct surface_t * s)
{
	s->r->shape_save(s);
}

static inline void surface_shape_restore(struct surface_t * s)
{
	s->r->shape_restore(s);
}

static inline void surface_shape_set_source(struct surface_t * s, struct surface_t * o, float x, float y)
{
	s->r->shape_set_source(s, o, x, y);
}

static inline void surface_shape_set_source_color(struct surface_t * s, struct color_t * c)
{
	s->r->shape_set_source_color(s, c);
}

static inline void surface_shape_set_fill_rule(struct surface_t * s, const char * t)
{
	s->r->shape_set_fill_rule(s, t);
}

static inline void surface_shape_set_line_width(struct surface_t * s, float w)
{
	s->r->shape_set_line_width(s, w);
}

static inline void surface_shape_set_line_cap(struct surface_t * s, const char * t)
{
	s->r->shape_set_line_cap(s, t);
}

static inline void surface_shape_set_line_join(struct surface_t * s, const char * t)
{
	s->r->shape_set_line_join(s, t);
}

static inline void surface_shape_set_miter_limit(struct surface_t * s, float l)
{
	s->r->shape_set_miter_limit(s, l);
}

static inline void surface_shape_set_dash(struct surface_t * s, float * dashes, int ndash, float offset)
{
	s->r->shape_set_dash(s, dashes, ndash, offset);
}

static inline void surface_shape_translate(struct surface_t * s, float tx, float ty)
{
	s->r->shape_translate(s, tx, ty);
}

static inline void surface_shape_scale(struct surface_t * s, float sx, float sy)
{
	s->r->shape_scale(s, sx, sy);
}

static inline void surface_shape_rotate(struct surface_t * s, float r)
{
	s->r->shape_rotate(s, r);
}

static inline void surface_shape_transform(struct surface_t * s, struct matrix2d_t * m)
{
	s->r->shape_transform(s, m);
}

static inline void surface_shape_set_matrix(struct surface_t * s, struct matrix2d_t * m)
{
	s->r->shape_set_matrix(s, m);
}

static inline void surface_shape_identity_matrix(struct surface_t * s)
{
	s->r->shape_identity_matrix(s);
}

static inline void surface_shape_new_path(struct surface_t * s)
{
	s->r->shape_new_path(s);
}

static inline void surface_shape_close_path(struct surface_t * s)
{
	s->r->shape_close_path(s);
}

static inline void surface_shape_move_to(struct surface_t * s, float x, float y)
{
	s->r->shape_move_to(s, x, y);
}

static inline void surface_shape_line_to(struct surface_t * s, float x, float y)
{
	s->r->shape_line_to(s, x, y);
}

static inline void surface_shape_curve_to(struct surface_t * s, float x1, float y1, float x2, float y2, float x3, float y3)
{
	s->r->shape_curve_to(s, x1, y1, x2, y2, x3, y3);
}

static inline void surface_shape_rectangle(struct surface_t * s, float x, float y, float w, float h)
{
	s->r->shape_rectangle(s, x, y, w, h);
}

static inline void surface_shape_round_rectangle(struct surface_t * s, float x, float y, float w, float h, float r)
{
	s->r->shape_round_rectangle(s, x, y, w, h, r);
}

static inline void surface_shape_arc(struct surface_t * s, float cx, float cy, float r, float a0, float a1)
{
	s->r->shape_arc(s, cx, cy, r, a0, a1);
}

static inline void surface_shape_arc_negative(struct surface_t * s, float cx, float cy, float r, float a0, float a1)
{
	s->r->shape_arc_negative(s, cx, cy, r, a0, a1);
}

static inline void surface_shape_circle(struct surface_t * s, float cx, float cy, float r)
{
	s->r->shape_circle(s, cx, cy, r);
}

static inline void surface_shape_ellipse(struct surface_t * s, float cx, float cy, float rx, float ry)
{
	s->r->shape_ellipse(s, cx, cy, rx, ry);
}

static inline void surface_shape_clip(struct surface_t * s)
{
	s->r->shape_clip(s);
}

static inline void surface_shape_clip_preserve(struct surface_t * s)
{
	s->r->shape_clip_preserve(s);
}

static inline void surface_shape_fill(struct surface_t * s)
{
	s->r->shape_fill(s);
}

static inline void surface_shape_fill_preserve(struct surface_t * s)
{
	s->r->shape_fill_preserve(s);
}

static inline void surface_shape_stroke(struct surface_t * s)
{
	s->r->shape_stroke(s);
}

static inline void surface_shape_stroke_preserve(struct surface_t * s)
{
	s->r->shape_stroke_preserve(s);
}

static inline void surface_shape_paint(struct surface_t * s)
{
	s->r->shape_paint(s);
}

struct render_t * search_render(void);
int register_render(struct render_t * r);
int unregister_render(struct render_t * r);
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

void surface_effect_glass(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h, int radius);
void surface_effect_shadow(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h, int radius, struct color_t * c);
void surface_effect_gradient(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h, struct color_t * lt, struct color_t * rt, struct color_t * rb, struct color_t * lb);
void surface_effect_checkerboard(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h);
void surface_filter_gray(struct surface_t * s);
void surface_filter_sepia(struct surface_t * s);
void surface_filter_invert(struct surface_t * s);
void surface_filter_coloring(struct surface_t * s, struct color_t * c);
void surface_filter_gamma(struct surface_t * s, float gamma);
void surface_filter_hue(struct surface_t * s, int angle);
void surface_filter_saturate(struct surface_t * s, int saturate);
void surface_filter_brightness(struct surface_t * s, int brightness);
void surface_filter_contrast(struct surface_t * s, int contrast);
void surface_filter_opacity(struct surface_t * s, int alpha);
void surface_filter_haldclut(struct surface_t * s, struct surface_t * clut, const char * type);
void surface_filter_blur(struct surface_t * s, int radius);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_GRAPHIC_SURFACE_H__ */
