#ifndef __XSTAR_KERNEL_VISION_RESIZE_H__
#define __XSTAR_KERNEL_VISION_RESIZE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/vision/vision.h>

struct vision_t * vision_resize(struct vision_t * v, int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_VISION_RESIZE_H__ */
