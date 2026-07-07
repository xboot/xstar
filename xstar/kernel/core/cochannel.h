#ifndef __XSTAR_KERNEL_CORE_COCHANNEL_H__
#define __XSTAR_KERNEL_CORE_COCHANNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <kernel/core/coroutine.h>

struct cochannel_t {
	unsigned char * buffer;
	unsigned int size;
	unsigned int in;
	unsigned int out;
};

struct cochannel_t * cochannel_alloc(unsigned int size);
void cochannel_free(struct cochannel_t * c);
void cochannel_send(struct scheduler_t * sched, struct cochannel_t * c, unsigned char * buf, unsigned int len);
void cochannel_recv(struct scheduler_t * sched, struct cochannel_t * c, unsigned char * buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_COCHANNEL_H__ */
