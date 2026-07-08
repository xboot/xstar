#ifndef __XSTARCFG_H__
#define __XSTARCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <setjmp.h>
#include <exit.h>
#include <errno.h>
#include <environ.h>
#include <locale.h>
#include <time.h>
#include <math.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>

typedef unsigned long io_addr_t;

struct thread_t {
	TaskHandle_t handle;
	void (*func)(void *);
	void * data;
	SemaphoreHandle_t sem;
};

struct mutex_t {
	SemaphoreHandle_t mutex;
};

struct semaphore_t {
	SemaphoreHandle_t sem;
};

#ifdef __cplusplus
}
#endif

#endif /* __XSTARCFG_H__ */
