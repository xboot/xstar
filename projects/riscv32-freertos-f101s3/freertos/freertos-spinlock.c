#include <freertos/freertos.h>

void freertos_spinlock_init(struct spinlock_t * lock)
{
	xatomic_store(&lock->lock, 0);
}

void freertos_spinlock_exit(struct spinlock_t * lock)
{
}

int freertos_spinlock_lock(struct spinlock_t * lock)
{
	taskENTER_CRITICAL();
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	while(xatomic_xchg(&lock->lock, 1));
	return 1;
}

int freertos_spinlock_trylock(struct spinlock_t * lock)
{
	taskENTER_CRITICAL();
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	if(xatomic_xchg(&lock->lock, 1))
	{
		taskEXIT_CRITICAL();
		return 0;
	}
	return 1;
}

int freertos_spinlock_unlock(struct spinlock_t * lock)
{
	__asm__ __volatile__ ("fence iorw, iorw" ::: "memory");
	xatomic_store_release(&lock->lock, 0);
	taskEXIT_CRITICAL();
	return 1;
}
