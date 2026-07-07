#ifndef __XSTAR_KERNEL_CORE_THWORKER_H__
#define __XSTAR_KERNEL_CORE_THWORKER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct thworker_t {
	struct list_head_t head;
	struct mutex_t lock;
	struct semaphore_t sem;
	struct thread_t * thread;
	int running;
};

struct thworker_t * thworker_alloc(const char * name);
void thworker_free(struct thworker_t * w);
void thworker_wait(struct thworker_t * w);
void thworker_clear(struct thworker_t * w, void (*cb)(void (*func)(void *), void * data));
void thworker_submit(struct thworker_t * w, void (*func)(void *), void * data);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_THWORKER_H__ */
