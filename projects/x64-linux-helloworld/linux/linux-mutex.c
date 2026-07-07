#include <linux/linux.h>

void linux_mutex_init(struct mutex_t * lock)
{
	if(lock)
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
		pthread_mutex_init(&lock->mutex, &attr);
		pthread_mutexattr_destroy(&attr);
	}
}

void linux_mutex_exit(struct mutex_t * lock)
{
	if(lock)
		pthread_mutex_destroy(&lock->mutex);
}

int linux_mutex_lock(struct mutex_t * lock)
{
	if(lock)
		return (pthread_mutex_lock(&lock->mutex) == 0) ? 1 : 0;
	return 0;
}

int linux_mutex_trylock(struct mutex_t * lock)
{
	if(lock)
		return (pthread_mutex_trylock(&lock->mutex) == 0) ? 1 : 0;
	return 0;
}

int linux_mutex_unlock(struct mutex_t * lock)
{
	if(lock)
		return (pthread_mutex_unlock(&lock->mutex) == 0) ? 1 : 0;
	return 0;
}
