#ifndef __XSTAR_KERNEL_TIME_DELAYCALL_H__
#define __XSTAR_KERNEL_TIME_DELAYCALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

void delaycall(unsigned int ms, void (*func)(void *), void * data);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_TIME_DELAYCALL_H__ */
