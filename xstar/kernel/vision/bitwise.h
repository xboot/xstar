#ifndef __XSTAR_KERNEL_VISION_BITWISE_H__
#define __XSTAR_KERNEL_VISION_BITWISE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/vision/vision.h>

void vision_bitwise_and(struct vision_t * v, struct vision_t * o);
void vision_bitwise_or(struct vision_t * v, struct vision_t * o);
void vision_bitwise_xor(struct vision_t * v, struct vision_t * o);
void vision_bitwise_not(struct vision_t * v);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_VISION_BITWISE_H__ */
