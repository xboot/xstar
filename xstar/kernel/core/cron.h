#ifndef __XSTAR_KERNEL_CORE_CRON_H__
#define __XSTAR_KERNEL_CORE_CRON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct cron_t {
	struct list_head_t head;
	struct mutex_t lock;
	struct semaphore_t sem;
	struct thread_t * thread;
	char * tz;
	int running;
};

struct cron_t * cron_alloc(const char * tz);
void cron_free(struct cron_t * cron);
int cron_add(struct cron_t * cron, const char * name, const char * expr, int oneshot, void (*func)(void *), void * data);
int cron_remove(struct cron_t * cron, const char * name);
void cron_foreach(struct cron_t * cron, void (*cb)(char * name, int oneshot, void * data), void * data);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_CRON_H__ */
