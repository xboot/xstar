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
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>
#include <sys/timerfd.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/rtc.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <alsa/asoundlib.h>

typedef unsigned long io_addr_t;

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
