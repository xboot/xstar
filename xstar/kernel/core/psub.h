#ifndef __XSTAR_KERNEL_CORE_PSUB_H__
#define __XSTAR_KERNEL_CORE_PSUB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

void psub_publish(const char * topic, void * pdat);
void psub_subscribe(const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
void psub_unsubscribe(const char * topic, void (*cb)(void *, void *), void * sdat);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_PSUB_H__ */
