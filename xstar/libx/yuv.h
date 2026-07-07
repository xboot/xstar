#ifndef __XSTAR_LIBX_YUV_H__
#define __XSTAR_LIBX_YUV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

void bt601_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b);
void bt601_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v);

void bt709_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b);
void bt709_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_YUV_H__ */
