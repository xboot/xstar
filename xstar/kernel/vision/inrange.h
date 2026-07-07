#ifndef __XSTAR_KERNEL_VISION_INRANGE_H__
#define __XSTAR_KERNEL_VISION_INRANGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/vision/vision.h>

struct vision_t * vision_inrange(struct vision_t * v, float * lower, float * upper);


#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_VISION_INRANGE_H__ */
