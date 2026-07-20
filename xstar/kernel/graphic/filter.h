#ifndef __XSTAR_KERNEL_GRAPHIC_FILTER_H__
#define __XSTAR_KERNEL_GRAPHIC_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/graphic/surface.h>

struct surface_t;

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

#endif /* __XSTAR_KERNEL_GRAPHIC_FILTER_H__ */
