#include <freertos/freertos.h>

static void thread_func_wrapper(void * arg)
{
	struct thread_t * thread = (struct thread_t*)arg;

	if(thread->func)
		thread->func(thread->data);
	xSemaphoreGive(thread->sem);
	vTaskDelete(NULL);
}

struct thread_t * freertos_thread_create(const char * name, void (*func)(void *), void * data, int stksz)
{
	struct thread_t * thread;

	thread = pvPortMalloc(sizeof(struct thread_t));
	if(!thread)
		return NULL;

	xos_memset(thread, 0, sizeof(struct thread_t));
	thread->func = func;
	thread->data = data;
	thread->sem = xSemaphoreCreateBinary();
	if(!thread->sem)
	{
		vPortFree(thread);
		return NULL;
	}
	if(stksz <= 0)
		stksz = 65536;
	int ret = xTaskCreate(thread_func_wrapper, name, (configSTACK_DEPTH_TYPE)((stksz + sizeof(StackType_t) - 1) / sizeof(StackType_t)), thread, configMAX_PRIORITIES / 2, &thread->handle);
	if(ret != pdPASS)
	{
		vSemaphoreDelete(thread->sem);
		vPortFree(thread);
		return NULL;
	}
	return thread;
}

void freertos_thread_destroy(struct thread_t *thread)
{
	if(thread)
	{
		freertos_thread_wait(thread);
		vPortFree(thread);
	}
}

void freertos_thread_wait(struct thread_t *thread)
{
	if(thread && thread->sem)
	{
		xSemaphoreTake(thread->sem, portMAX_DELAY);
		vSemaphoreDelete(thread->sem);
		thread->sem = NULL;
	}
}

void freertos_thread_sleep(uint64_t ns)
{
	TickType_t ticks = pdMS_TO_TICKS((ns + 999999) / 1000000);
	ticks = ticks > 0 ? ticks : 1;
	vTaskDelay(ticks);
}
