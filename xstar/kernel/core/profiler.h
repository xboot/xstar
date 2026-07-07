#ifndef __XSTAR_KERNEL_CORE_PROFILER_H__
#define __XSTAR_KERNEL_CORE_PROFILER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/clocksource/clocksource.h>

struct profiler_t {
	uint64_t begin;
	uint64_t end;
	uint64_t elapsed;
	uint64_t count;
};

static inline void profiler_begin(struct profiler_t * p)
{
	if(p)
		p->begin = ktime_to_ns(ktime_get());
}

static inline void profiler_end(struct profiler_t * p)
{
	if(p)
	{
		p->end = ktime_to_ns(ktime_get());
		p->elapsed += p->end - p->begin;
		p->count++;
	}
}

struct profiler_t * profiler_search(const char * name);
void profiler_foreach(void (*cb)(const char * name, uint64_t count, uint64_t time));
void profiler_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_CORE_PROFILER_H__ */
