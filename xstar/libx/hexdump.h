#ifndef __XSTAR_LIBX_HEXDUMP_H__
#define __XSTAR_LIBX_HEXDUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

void hexdump(void (*output)(char), unsigned long base, void * buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_HEXDUMP_H__ */
