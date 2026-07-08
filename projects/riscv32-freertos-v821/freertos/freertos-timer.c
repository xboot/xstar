#include <freertos/freertos.h>

struct freertos_timer_context_t {
	struct {
		void (*cb)(void *);
		void * data;
	} tcd;
	TimerHandle_t timer;
};
static struct freertos_timer_context_t tctx;

static void timer_handler(TimerHandle_t timer)
{
	if(tctx.tcd.cb)
		tctx.tcd.cb(tctx.tcd.data);
}

void freertos_timer_init(void)
{
	tctx.tcd.cb = NULL;
	tctx.tcd.data = NULL;
	tctx.timer = xTimerCreate("timer", pdMS_TO_TICKS(10), pdFALSE, NULL, timer_handler);
}

void freertos_timer_exit(void)
{
    xTimerStop(tctx.timer, portMAX_DELAY);
	xTimerDelete(tctx.timer, portMAX_DELAY);
}

void freertos_timer_next(uint64_t time, void (*cb)(void *), void * data)
{
	tctx.tcd.cb = cb;
	tctx.tcd.data = data;
	xTimerChangePeriod(tctx.timer, pdMS_TO_TICKS(time / 1000000ULL), portMAX_DELAY);
	if(xTimerIsTimerActive(tctx.timer) == pdFALSE)
		xTimerStart(tctx.timer, 0);
}

uint64_t freertos_timer_count(void)
{
	return (uint64_t)(pdTICKS_TO_MS(xTaskGetTickCount()) * 1000000ULL);
}

uint64_t freertos_timer_frequency(void)
{
	return 1000000000ULL;
}
