#include <win/win.h>

static void * thread_func_wrapper(void * arg)
{
	struct thread_t * thread = (struct thread_t *)arg;

	if(thread->func)
		thread->func(thread->data);
	pthread_mutex_lock(&thread->mutex);
	thread->finished = 1;
	pthread_cond_broadcast(&thread->cond);
	pthread_mutex_unlock(&thread->mutex);

	return NULL;
}

struct thread_t * win_thread_create(const char * name, void (*func)(void *), void * data, int stksz)
{
	struct thread_t * thread;

	thread = malloc(sizeof(struct thread_t));
	if(!thread)
		return NULL;

	memset(thread, 0, sizeof(struct thread_t));
	thread->func = func;
	thread->data = data;
	thread->finished = 0;
	pthread_cond_init(&thread->cond, NULL);
	pthread_mutex_init(&thread->mutex, NULL);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if(stksz > 0)
		pthread_attr_setstacksize(&attr, stksz);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int ret = pthread_create(&thread->thread, &attr, thread_func_wrapper, thread);
	pthread_attr_destroy(&attr);
	if(ret != 0)
	{
		pthread_mutex_destroy(&thread->mutex);
		pthread_cond_destroy(&thread->cond);
		free(thread);
		return NULL;
	}
	if(name && (name[0] != '\0'))
	{
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%s", name);
        pthread_setname_np(thread->thread, tmp);
	}
	return thread;
}

void win_thread_destroy(struct thread_t * thread)
{
	if(thread)
	{
		win_thread_wait(thread);
		pthread_mutex_destroy(&thread->mutex);
		pthread_cond_destroy(&thread->cond);
		free(thread);
	}
}

void win_thread_wait(struct thread_t * thread)
{
	if(thread)
	{
		pthread_mutex_lock(&thread->mutex);
		while(!thread->finished)
			pthread_cond_wait(&thread->cond, &thread->mutex);
		pthread_mutex_unlock(&thread->mutex);
	}
}

void win_thread_sleep(uint64_t ns)
{
	struct timespec ts;

	ts.tv_sec = ns / 1000000000ULL;
	ts.tv_nsec = ns % 1000000000ULL;
	nanosleep(&ts, NULL);
}
