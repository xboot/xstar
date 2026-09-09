#include <win/win.h>

void win_spinlock_init(struct spinlock_t * lock)
{
	pthread_spin_init(&lock->lock, PTHREAD_PROCESS_PRIVATE);
}

void win_spinlock_exit(struct spinlock_t * lock)
{
	pthread_spin_destroy(&lock->lock);
}

int win_spinlock_lock(struct spinlock_t * lock)
{
	return (pthread_spin_lock(&lock->lock) == 0) ? 1 : 0;
}

int win_spinlock_trylock(struct spinlock_t * lock)
{
	return (pthread_spin_trylock(&lock->lock) == 0) ? 1 : 0;
}

int win_spinlock_unlock(struct spinlock_t * lock)
{
	return (pthread_spin_unlock(&lock->lock) == 0) ? 1 : 0;
}
