#ifndef __XSTAR_KERNEL_VISION_RECTANGLE_H__
#define __XSTAR_KERNEL_VISION_RECTANGLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/vision/vision.h>

void vision_rectangle(struct vision_t * v, int x, int y, int w, int h, int thickness, struct color_t * c);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_VISION_RECTANGLE_H__ */
