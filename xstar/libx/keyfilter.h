#ifndef __XSTAR_LIBX_KEYFILTER_H__
#define __XSTAR_LIBX_KEYFILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/ktime.h>

enum keyfilter_state_t {
	KEYFILTER_STATE_IDLE		= 0,
	KEYFILTER_STATE_PRESSED		= 1,
	KEYFILTER_STATE_RELEASED	= 2,
	KEYFILTER_STATE_REPRESSED	= 3,
	KEYFILTER_STATE_HELD		= 4,
};

enum keyfilter_type_t {
	KEYFILTER_TYPE_NONE			= 0,
	KEYFILTER_TYPE_DOWN			= 1,
	KEYFILTER_TYPE_UP			= 2,
	KEYFILTER_TYPE_CLICK		= 3,
	KEYFILTER_TYPE_DBLCLICK		= 4,
	KEYFILTER_TYPE_LONGPRESS	= 5,
	KEYFILTER_TYPE_REPEAT		= 6,
};

struct keyfilter_entry_t {
	ktime_t timestamp;
	int pressed;
};

struct keyfilter_t {
	struct keyfilter_entry_t queue[8];
	int head, tail;
	int pressed;
	struct {
		int click;
		int longpress;
		int repeat;
	} timeout;
	ktime_t timestamp;
	ktime_t repeatlast;
	enum keyfilter_state_t state;
	enum keyfilter_type_t pending;
};

void keyfilter_init(struct keyfilter_t * filter);
void keyfilter_settimeout(struct keyfilter_t * filter, int click, int longpress, int repeat);
int keyfilter_feed(struct keyfilter_t * filter, ktime_t timestamp, int pressed);
enum keyfilter_type_t keyfilter_poll(struct keyfilter_t * filter, ktime_t now);
void keyfilter_clear(struct keyfilter_t * filter);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_KEYFILTER_H__ */
