#include <win/win.h>

void win_semaphore_init(struct semaphore_t * sem, uint32_t count)
{
	if(sem)
		sem_init(&sem->sem, 0, count);
}

void win_semaphore_exit(struct semaphore_t * sem)
{
	if(sem)
		sem_destroy(&sem->sem);
}

int win_semaphore_wait(struct semaphore_t * sem, uint32_t timeout)
{
	if(sem)
	{
		if(timeout > 0)
		{
			struct timespec ts = { 0, 0 };
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += timeout / 1000;
			ts.tv_nsec += ((timeout % 1000) * 1000000);
			if(ts.tv_nsec >= 1000000000ULL)
			{
				ts.tv_sec += ts.tv_nsec / 1000000000ULL;
				ts.tv_nsec = ts.tv_nsec % 1000000000ULL;
			}
			return (sem_timedwait(&sem->sem, &ts) == 0) ? 1 : 0;
		}
		return (sem_wait(&sem->sem) == 0) ? 1 : 0;
	}
	return 0;
}

int win_semaphore_post(struct semaphore_t * sem)
{
	if(sem)
		return (sem_post(&sem->sem) == 0) ? 1 : 0;
	return 0;
}
