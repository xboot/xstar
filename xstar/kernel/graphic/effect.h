#ifndef __XSTAR_KERNEL_GRAPHIC_EFFECT_H__
#define __XSTAR_KERNEL_GRAPHIC_EFFECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/graphic/surface.h>

struct surface_t;

void surface_effect_glass(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h, int radius);
void surface_effect_shadow(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h, int radius, struct color_t * c);
void surface_effect_gradient(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h, struct color_t * lt, struct color_t * rt, struct color_t * rb, struct color_t * lb);
void surface_effect_checkerboard(struct surface_t * s, struct region_t * clip, int x, int y, int w, int h);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_GRAPHIC_EFFECT_H__ */
