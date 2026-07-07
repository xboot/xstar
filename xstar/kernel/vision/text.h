#ifndef __XSTAR_KERNEL_VISION_TEXT_H__
#define __XSTAR_KERNEL_VISION_TEXT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/vision/vision.h>

void vision_text(struct vision_t * v, int x, int y, int wrap, const char * family, enum font_style_t style, int size, struct color_t * c, const char * fmt, ...);
void vision_icon(struct vision_t * v, int x, int y, const char * family, int size, uint32_t code, struct color_t * c);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_VISION_TEXT_H__ */
