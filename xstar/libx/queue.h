#ifndef __XSTAR_LIBX_QUEUE_H__
#define __XSTAR_LIBX_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/list.h>

struct queue_node_t {
	struct list_head_t entry;
	void * data;
};

struct queue_t {
	struct list_head_t head;
};

struct queue_t * queue_alloc(void);
void queue_free(struct queue_t * q, void (*cb)(void *));
void queue_clear(struct queue_t * q, void (*cb)(void *));

int queue_isempty(struct queue_t * q);
void queue_push(struct queue_t * q, void * data);
void * queue_pop(struct queue_t * q);
void * queue_peek(struct queue_t * q);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_QUEUE_H__ */
