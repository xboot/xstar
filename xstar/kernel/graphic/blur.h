#ifndef __XSTAR_KERNEL_GRAPHIC_BLUR_H__
#define __XSTAR_KERNEL_GRAPHIC_BLUR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

void blur(unsigned char * pixel, int width, int height, int x, int y, int w, int h, int radius);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_GRAPHIC_BLUR_H__ */
