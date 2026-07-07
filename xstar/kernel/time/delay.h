#ifndef __XSTAR_KERNEL_TIME_DELAY_H__
#define __XSTAR_KERNEL_TIME_DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

void ndelay(uint32_t ns);
void udelay(uint32_t us);
void mdelay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_TIME_DELAY_H__ */
