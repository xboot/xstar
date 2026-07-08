#include <freertos/freertos.h>

void freertos_semaphore_init(struct semaphore_t * sem, uint32_t count)
{
	if(sem)
		sem->sem = xSemaphoreCreateCounting(1000, count);
}

void freertos_semaphore_exit(struct semaphore_t * sem)
{
	if(sem)
		vSemaphoreDelete(sem->sem);
}

int freertos_semaphore_wait(struct semaphore_t * sem, uint32_t timeout)
{
	if(sem)
	{
		if(timeout > 0)
		{
			TickType_t ticks = pdMS_TO_TICKS(timeout);
			return (xSemaphoreTake(sem->sem, ticks) == pdTRUE) ? 1 : 0;
		}
		return (xSemaphoreTake(sem->sem, portMAX_DELAY) == pdTRUE) ? 1 : 0;
	}
	return 0;
}

int freertos_semaphore_post(struct semaphore_t * sem)
{
	if(sem)
		return (xSemaphoreGive(sem->sem) == pdTRUE) ? 1 : 0;
	return 0;
}
