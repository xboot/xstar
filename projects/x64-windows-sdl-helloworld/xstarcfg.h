#ifndef __XSTARCFG_H__
#define __XSTARCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <malloc.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <direct.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <io.h>
#include <conio.h>
#include <windows.h>
#include <SDL3/SDL.h>

typedef unsigned long long io_addr_t;

struct thread_t {
	pthread_t thread;
	void (*func)(void *);
	void * data;
	int finished;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

struct mutex_t {
	pthread_mutex_t mutex;
};

struct semaphore_t {
	sem_t sem;
};

#ifdef __cplusplus
}
#endif

#endif /* __XSTARCFG_H__ */
