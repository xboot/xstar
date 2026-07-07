#ifndef __XSTAR_LIBX_FIFO_H__
#define __XSTAR_LIBX_FIFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/log2.h>

struct fifo_t {
	unsigned char * buffer;
	unsigned int size;
	unsigned int in;
	unsigned int out;
	struct mutex_t lock;
};

void __fifo_reset(struct fifo_t * f);
int __fifo_isempty(struct fifo_t * f);
int __fifo_isfull(struct fifo_t * f);
unsigned int __fifo_size(struct fifo_t * f);
unsigned int __fifo_length(struct fifo_t * f);
unsigned int __fifo_available(struct fifo_t * f);
unsigned int __fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int __fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int __fifo_peek(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int __fifo_put_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int  __fifo_get_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);

struct fifo_t * fifo_alloc(unsigned int size);
void fifo_free(struct fifo_t * f);
void fifo_reset(struct fifo_t * f);
int fifo_isempty(struct fifo_t * f);
int fifo_isfull(struct fifo_t * f);
unsigned int fifo_size(struct fifo_t * f);
unsigned int fifo_length(struct fifo_t * f);
unsigned int fifo_available(struct fifo_t * f);
unsigned int fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_peek(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_put_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);
unsigned int fifo_get_packet(struct fifo_t * f, unsigned char * buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_FIFO_H__ */
