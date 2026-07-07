#ifndef __XSTAR_KERNEL_VISION_THRESHOLD_H__
#define __XSTAR_KERNEL_VISION_THRESHOLD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/vision/vision.h>

void vision_threshold(struct vision_t * v, int threshold, const char * type);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_VISION_THRESHOLD_H__ */
