#ifndef __XSTAR_KERNEL_VISION_DITHER_H__
#define __XSTAR_KERNEL_VISION_DITHER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/vision/vision.h>

void vision_dither(struct vision_t * v, const char * type);
void vision_dither_palette(struct vision_t * v, struct color_t * palette, int n);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_VISION_DITHER_H__ */
