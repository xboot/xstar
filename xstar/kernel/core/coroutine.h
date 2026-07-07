#ifndef __XSTAR_KERNEL_CORE_COROUTINE_H__
#define __XSTAR_KERNEL_CORE_COROUTINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct coroutine_t;
struct scheduler_t;

struct coroutine_t {
	struct scheduler_t * sched;
	struct list_head_t list;
	void * fctx;
	void * stack;
	void (*func)(struct scheduler_t *, void *);
	void * data;
};

struct scheduler_t {
	struct list_head_t ready;
	struct coroutine_t * running;
	void * fctx;
};

static inline struct coroutine_t * coroutine_self(struct scheduler_t * sched)
{
	return sched->running;
}

struct coroutine_t * coroutine_start(struct scheduler_t * sched, void (*func)(struct scheduler_t *, void *), void * data, size_t stksz);
void coroutine_yield(struct scheduler_t * sched);
void coroutine_nsleep(struct scheduler_t * sched, uint32_t ns);
void coroutine_usleep(struct scheduler_t * sched, uint32_t us);
void coroutine_msleep(struct scheduler_t * sched, uint32_t ms);

void scheduler_init(struct scheduler_t * sched);
void scheduler_loop(struct scheduler_t * sched);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_COROUTINE_H__ */
