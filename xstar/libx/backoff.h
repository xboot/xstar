#ifndef __XSTAR_LIBX_BACKOFF_H__
#define __XSTAR_LIBX_BACKOFF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

struct backoff_t {
	int max_delay;
	int max_retry;
	int delay;
	int retry;
};

void backoff_init(struct backoff_t * ctx, int delay, int max_delay, int max_retry);
int backoff_next(struct backoff_t * ctx);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_BACKOFF_H__ */
