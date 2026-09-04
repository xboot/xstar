#ifndef __XSTAR_KERNEL_CORE_THMAILBOX_H__
#define __XSTAR_KERNEL_CORE_THMAILBOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct thmailbox_t {
	unsigned char * pool;
	unsigned int block_size;
	unsigned int block_count;
	void * freelist;
	unsigned int freecount;
	void ** ring;
	unsigned int ring_size;
	unsigned int in;
	unsigned int out;
	struct mutex_t lock;
	struct semaphore_t ssem;
	struct semaphore_t rsem;
	struct semaphore_t psem;
};

struct thmailbox_t * thmailbox_alloc(unsigned int size, unsigned int count);
void thmailbox_free(struct thmailbox_t * m);
void thmailbox_reset(struct thmailbox_t * m);

void * thmailbox_buf_alloc(struct thmailbox_t * m, unsigned int len, int timeout);
void thmailbox_buf_free(struct thmailbox_t * m, void * buf);
unsigned int thmailbox_buf_available(struct thmailbox_t * m);

unsigned int thmailbox_send(struct thmailbox_t * m, void * buf, unsigned int len, int timeout);
unsigned int thmailbox_recv(struct thmailbox_t * m, void ** pbuf, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_THMAILBOX_H__ */
