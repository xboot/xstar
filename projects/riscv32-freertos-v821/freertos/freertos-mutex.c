#include <freertos/freertos.h>

void freertos_mutex_init(struct mutex_t * lock)
{
	if(lock)
	{
#if configUSE_RECURSIVE_MUTEXES
		lock->mutex = xSemaphoreCreateRecursiveMutex();
#else
		lock->mutex = xSemaphoreCreateMutex();
#endif
	}
}

void freertos_mutex_exit(struct mutex_t * lock)
{
	if(lock)
		vSemaphoreDelete(lock->mutex);
}

int freertos_mutex_lock(struct mutex_t * lock)
{
	if(lock)
	{
#if configUSE_RECURSIVE_MUTEXES
		return (xSemaphoreTakeRecursive(lock->mutex, portMAX_DELAY) == pdTRUE) ? 1 : 0;
#else
		return (xSemaphoreTake(lock->mutex, portMAX_DELAY) == pdTRUE) ? 1 : 0;
#endif
	}
	return 0;
}

int freertos_mutex_trylock(struct mutex_t * lock)
{
	if(lock)
	{
#if configUSE_RECURSIVE_MUTEXES
		return (xSemaphoreTakeRecursive(lock->mutex, 0) == pdTRUE) ? 1 : 0;
#else
		return (xSemaphoreTake(lock->mutex, 0) == pdTRUE) ? 1 : 0;
#endif
	}
	return 0;
}

int freertos_mutex_unlock(struct mutex_t * lock)
{
	if(lock)
	{
#if configUSE_RECURSIVE_MUTEXES
		return (xSemaphoreGiveRecursive(lock->mutex) == pdTRUE) ? 1 : 0;
#else
		return (xSemaphoreGive(lock->mutex) == pdTRUE) ? 1 : 0;
#endif
	}
	return 0;
}
