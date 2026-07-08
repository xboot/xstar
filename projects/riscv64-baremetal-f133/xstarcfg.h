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

typedef unsigned long long io_addr_t;

struct thread_t {
	volatile int thread;
};

struct mutex_t {
	volatile int mutex;
};

struct semaphore_t {
	volatile int sem;
};

#ifdef __cplusplus
}
#endif

#endif /* __XSTARCFG_H__ */
