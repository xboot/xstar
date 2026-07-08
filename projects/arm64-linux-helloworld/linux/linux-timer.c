#include <linux/linux.h>

struct linux_timer_context_t {
	timer_t tid;
	void (*cb)(void *);
	void * data;
	int running;
	sem_t semaphore;
	pthread_mutex_t mutex;
	pthread_t thread;
};
static struct linux_timer_context_t tctx;

static void signal_timer_handler(int signum)
{
	sem_post(&tctx.semaphore);
}

static void * signal_thread_func(void * arg)
{
	while(tctx.running)
	{
		sem_wait(&tctx.semaphore);
		if(tctx.cb)
			tctx.cb(tctx.data);
	}
	return NULL;
}

void linux_timer_init(void)
{
	struct itimerspec its;
	struct sigevent sev;

	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGUSR1;
	sev.sigev_value.sival_ptr = &tctx.tid;
	signal(SIGUSR1, signal_timer_handler);
	timer_create(CLOCK_MONOTONIC, &sev, &tctx.tid);
	tctx.cb = NULL;
	tctx.data = NULL;
	tctx.running = 1;
	timer_settime(tctx.tid, 0, &its, NULL);
	sem_init(&tctx.semaphore, 0, 0);
	pthread_mutex_init(&tctx.mutex, NULL);
	pthread_create(&tctx.thread, NULL, signal_thread_func, NULL);
}

void linux_timer_exit(void)
{
	pthread_mutex_lock(&tctx.mutex);
	{
		tctx.running = 0;
		sem_post(&tctx.semaphore);
		pthread_join(tctx.thread, NULL);
		timer_delete(tctx.tid);
		sem_destroy(&tctx.semaphore);
	}
	pthread_mutex_unlock(&tctx.mutex);
	pthread_mutex_destroy(&tctx.mutex);
}

void linux_timer_next(uint64_t time, void (*cb)(void *), void * data)
{
	struct itimerspec its;

	its.it_value.tv_sec = time / 1000000000ULL;
	its.it_value.tv_nsec = time % 1000000000ULL;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;
	pthread_mutex_lock(&tctx.mutex);
	{
		tctx.cb = cb;
		tctx.data = data;
		timer_settime(tctx.tid, 0, &its, NULL);
	}
	pthread_mutex_unlock(&tctx.mutex);
}

uint64_t linux_timer_count(void)
{
	struct timespec ts;

    if(clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
    	return 0;
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

uint64_t linux_timer_frequency(void)
{
	return 1000000000ULL;
}

uint64_t linux_realtime(void)
{
	struct timespec ts;

    if(clock_gettime(CLOCK_REALTIME, &ts) == -1)
    	return 0;
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
