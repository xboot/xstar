#ifndef __XSTAR_KERNEL_CORE_THCHANNEL_H__
#define __XSTAR_KERNEL_CORE_THCHANNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct thchannel_t {
	unsigned char * buffer;
	unsigned int size;
	unsigned int in;
	unsigned int out;
	struct mutex_t lock;
	struct semaphore_t ssem;
	struct semaphore_t rsem;
};

struct thchannel_t * thchannel_alloc(unsigned int size);
void thchannel_free(struct thchannel_t * c);
void thchannel_reset(struct thchannel_t * c);
int thchannel_isempty(struct thchannel_t * c);
int thchannel_isfull(struct thchannel_t * c);
unsigned int thchannel_size(struct thchannel_t * c);
unsigned int thchannel_length(struct thchannel_t * c);
unsigned int thchannel_available(struct thchannel_t * c);
unsigned int thchannel_send(struct thchannel_t * c, unsigned char * buf, unsigned int len, unsigned int timeout);
unsigned int thchannel_recv(struct thchannel_t * c, unsigned char * buf, unsigned int len, unsigned int timeout);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_THCHANNEL_H__ */
