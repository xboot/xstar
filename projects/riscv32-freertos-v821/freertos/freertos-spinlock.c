#include <freertos/freertos.h>

void freertos_spinlock_init(struct spinlock_t * lock)
{
	if(lock)
		lock->lock = 0;
}

void freertos_spinlock_exit(struct spinlock_t * lock)
{
}

int freertos_spinlock_lock(struct spinlock_t * lock)
{
	if(!lock)
		return 0;
	taskENTER_CRITICAL();
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	lock->lock = 1;
	return 1;
}

int freertos_spinlock_trylock(struct spinlock_t * lock)
{
	if(!lock)
		return 0;
	return freertos_spinlock_lock(lock);
}

int freertos_spinlock_unlock(struct spinlock_t * lock)
{
	if(!lock)
		return 0;
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	lock->lock = 0;
	taskEXIT_CRITICAL();
	return 1;
}
