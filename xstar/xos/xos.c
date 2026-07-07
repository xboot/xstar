/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <xos/xos.h>
#include <driver/console/console.h>

/*
 * mem
 */
static void * __xos_mem_malloc(size_t size)
{
	return NULL;
}

static void * __xos_mem_memalign(size_t align, size_t size)
{
	return NULL;
}

static void * __xos_mem_realloc(void * ptr, size_t size)
{
	return NULL;
}

static void * __xos_mem_calloc(size_t nmemb, size_t size)
{
	return NULL;
}

static void __xos_mem_free(void * ptr)
{
}

static void __xos_mem_meminfo(size_t * mused, size_t * mfree)
{
	if(mused)
		*mused = 0;
	if(mfree)
		*mfree = 0;
}

/*
 * dma
 */
static void * __xos_dma_alloc_coherent(unsigned long size)
{
	return xos_mem_memalign(4096, size);
}

static void __xos_dma_free_coherent(void * addr)
{
	xos_mem_free(addr);
}

static void * __xos_dma_alloc_noncoherent(unsigned long size)
{
	return xos_mem_memalign(4096, size);
}

static void __xos_dma_free_noncoherent(void * addr)
{
	xos_mem_free(addr);
}

static void __xos_dma_sync(void * addr, unsigned long size, int flag)
{
}

/*
 * io
 */
static uint8_t __xos_io_read8(io_addr_t addr)
{
	return (*((volatile uint8_t *)(addr)));
}

static void __xos_io_write8(io_addr_t addr, uint8_t value)
{
	*((volatile uint8_t *)(addr)) = value;
}

static uint16_t __xos_io_read16(io_addr_t addr)
{
	return (*((volatile uint16_t *)(addr)));
}

static void __xos_io_write16(io_addr_t addr, uint16_t value)
{
	*((volatile uint16_t *)(addr)) = value;
}

static uint32_t __xos_io_read32(io_addr_t addr)
{
	return (*((volatile uint32_t *)(addr)));
}

static void __xos_io_write32(io_addr_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

static uint64_t __xos_io_read64(io_addr_t addr)
{
	return (*((volatile uint64_t *)(addr)));
}

static void __xos_io_write64(io_addr_t addr, uint64_t value)
{
	*((volatile uint64_t *)(addr)) = value;
}

/*
 * stdio
 */
static ssize_t __xos_stdio_read(void * buf, size_t count)
{
	static struct console_t * con = NULL;

	if(!con)
		con = search_first_console();
	return console_read(con, buf, count);
}

static ssize_t __xos_stdio_write(void * buf, size_t count)
{
	static struct console_t * con = NULL;

	if(!con)
		con = search_first_console();
	return console_write(con, buf, count);
}

/*
 * pm
 */
static void __xos_pm_shutdown(void)
{
}

static void __xos_pm_reboot(void)
{
}

static void __xos_pm_standby(void)
{
}

/*
 * copyright
 */
static char * __xos_copyright_uniqueid(void)
{
	return "0123456789";
}

static int __xos_copyright_verify(void)
{
	return 1;
}

/*
 * file
 */
static char * __xos_file_cwd(void)
{
	return NULL;
}

static int __xos_file_open(const char * path, const char * mode)
{
	return 0;
}

static int __xos_file_close(int fd)
{
	return 0;
}

static int __xos_file_isdir(const char * path)
{
	return 0;
}

static int __xos_file_isfile(const char * path)
{
	return 0;
}

static int __xos_file_mode(const char * path)
{
	return 0;
}

static int __xos_file_mkdir(const char * path)
{
	return 0;
}

static int __xos_file_remove(const char * path)
{
	return 0;
}

static int __xos_file_access(const char * path, const char * mode)
{
	return 0;
}

static void __xos_file_walk(const char * path, void (*cb)(const char * dir, const char * name, void * data), const char * dir, void * data)
{
}

static ssize_t __xos_file_read(int fd, void * buf, size_t count)
{
	return 0;
}

static ssize_t __xos_file_write(int fd, const void * buf, size_t count)
{
	return 0;
}

static int64_t __xos_file_seek(int fd, int64_t offset)
{
	return 0;
}

static int64_t __xos_file_tell(int fd)
{
	return 0;
}

static int64_t __xos_file_length(int fd)
{
	return 0;
}

static void __xos_file_sync(int fd)
{
}

/*
 * coroutine
 */
static void * __xos_coroutine_make(void * stack, size_t size, void (*func)(struct co_transfer_t))
{
	return NULL;
}

static inline struct co_transfer_t __xos_coroutine_jump(void * fctx, void * priv)
{
	return (struct co_transfer_t){ NULL, NULL };
}

/*
 * thread
 */
static struct thread_t * __xos_thread_create(const char * name, void (*func)(void *), void * data, int stksz)
{
	return NULL;
}

static void __xos_thread_destroy(struct thread_t * thread)
{
}

static void __xos_thread_wait(struct thread_t * thread)
{
}

static void __xos_thread_sleep(uint64_t ns)
{
}

/*
 * mutex
 */
static void __xos_mutex_init(struct mutex_t * lock)
{
}

static void __xos_mutex_exit(struct mutex_t * lock)
{
}

static int __xos_mutex_lock(struct mutex_t * lock)
{
	return 1;
}

static int __xos_mutex_trylock(struct mutex_t * lock)
{
	return 1;
}

static int __xos_mutex_unlock(struct mutex_t * lock)
{
	return 1;
}

/*
 * semaphore
 */
static void __xos_semaphore_init(struct semaphore_t * sem, uint32_t count)
{
}

static void __xos_semaphore_exit(struct semaphore_t * sem)
{
}

static int __xos_semaphore_wait(struct semaphore_t * sem, uint32_t timeout)
{
	return 1;
}

static int __xos_semaphore_post(struct semaphore_t * sem)
{
	return 1;
}

/*
 * other
 */
static char * __xos_other_strcpy(char * dest, const char * src)
{
	char * tmp = dest;

	while((*dest++ = *src++) != '\0');
	return tmp;
}

static char * __xos_other_strncpy(char * dest, const char * src, size_t n)
{
	char * tmp = dest;

	while(n)
	{
		if((*tmp = *src) != 0)
			src++;
		tmp++;
		n--;
	}
	return dest;
}

static char * __xos_other_strcat(char * dest, const char * src)
{
	char * tmp = dest;

	while(*dest)
		dest++;
	while((*dest++ = *src++) != '\0');
	return tmp;
}

static char * __xos_other_strncat(char * dest, const char * src, size_t n)
{
	char * tmp = dest;

	if(n)
	{
		while(*dest)
			dest++;
		while((*dest++ = *src++) != 0)
		{
			if(--n == 0)
			{
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}

static size_t __xos_other_strlen(const char * s)
{
	const char * sc;

	for(sc = s; *sc != '\0'; ++sc);
	return sc - s;
}

static size_t __xos_other_strnlen(const char * s, size_t n)
{
	const char * sc;

	for(sc = s; n-- && *sc != '\0'; ++sc);
	return sc - s;
}

static int __xos_other_strcmp(const char * s1, const char * s2)
{
	int res;

	while(1)
	{
		if((res = *s1 - *s2++) != 0 || !*s1++)
			break;
	}
	return res;
}

static int __xos_other_strncmp(const char * s1, const char * s2, size_t n)
{
	int __res = 0;

	while(n)
	{
		if((__res = *s1 - *s2++) != 0 || !*s1++)
			break;
		n--;
	}
	return __res;
}

static int __xos_other_tolower(int c)
{
	if(((unsigned int)c - 'A') < 26)
		return c | 32;
	return c;
}

static int __xos_other_strcasecmp(const char * s1, const char * s2)
{
	int c1, c2;

	do {
		c1 = __xos_other_tolower(*s1++);
		c2 = __xos_other_tolower(*s2++);
	} while(c1 == c2 && c1 != 0);

	return c1 - c2;
}

static int __xos_other_strncasecmp(const char * s1, const char * s2, size_t n)
{
	int c1, c2;

	do {
		c1 = __xos_other_tolower(*s1++);
		c2 = __xos_other_tolower(*s2++);
	} while((--n > 0) && c1 == c2 && c1 != 0);

	return c1 - c2;
}

static void * __xos_other_memset(void * s, int c, size_t n)
{
	char * xs = s;

	while(n--)
		*xs++ = c;
	return s;
}

static void * __xos_other_memcpy(void * dest, const void * src, size_t len)
{
	char * tmp = dest;
	const char * s = src;

	while(len--)
		*tmp++ = *s++;
	return dest;
}

static void * __xos_other_memmove(void * dest, const void * src, size_t n)
{
	char * tmp;
	const char * s;

	if(dest <= src)
	{
		tmp = dest;
		s = src;
		while (n--)
			*tmp++ = *s++;
	}
	else
	{
		tmp = dest;
		tmp += n;
		s = src;
		s += n;
		while(n--)
			*--tmp = *--s;
	}
	return dest;
}

static void * __xos_other_memchr(const void * s, int c, size_t n)
{
	const unsigned char * p = s;

	while(n-- != 0)
	{
        if((unsigned char)c == *p++)
        {
			return (void *)(p - 1);
		}
	}
	return NULL;
}

static int __xos_other_memcmp(const void * s1, const void * s2, size_t n)
{
	const unsigned char * su1, * su2;
	int res = 0;

	for(su1 = s1, su2 = s2; 0 < n; ++su1, ++su2, n--)
	{
		if((res = *su1 - *su2) != 0)
			break;
	}
	return res;
}

struct xos_environ_t __xos_environ = {
	.mem = {
		.malloc = __xos_mem_malloc,
		.memalign = __xos_mem_memalign,
		.realloc = __xos_mem_realloc,
		.calloc = __xos_mem_calloc,
		.free = __xos_mem_free,
		.meminfo = __xos_mem_meminfo,
	},

	.dma = {
		.alloc_coherent = __xos_dma_alloc_coherent,
		.free_coherent = __xos_dma_free_coherent,
		.alloc_noncoherent = __xos_dma_alloc_noncoherent,
		.free_noncoherent = __xos_dma_free_noncoherent,
		.sync = __xos_dma_sync,
	},

	.io = {
		.read8 = __xos_io_read8,
		.write8 = __xos_io_write8,
		.read16 = __xos_io_read16,
		.write16 = __xos_io_write16,
		.read32 = __xos_io_read32,
		.write32 = __xos_io_write32,
		.read64 = __xos_io_read64,
		.write64 = __xos_io_write64,
	},

	.stdio = {
		.read = __xos_stdio_read,
		.write = __xos_stdio_write,
	},

	.pm = {
		.shutdown = __xos_pm_shutdown,
		.reboot = __xos_pm_reboot,
		.standby = __xos_pm_standby,
	},

	.copyright = {
		.uniqueid = __xos_copyright_uniqueid,
		.verify = __xos_copyright_verify,
	},

	.file = {
		.cwd = __xos_file_cwd,
		.open = __xos_file_open,
		.close = __xos_file_close,
		.isdir = __xos_file_isdir,
		.isfile = __xos_file_isfile,
		.mode = __xos_file_mode,
		.mkdir = __xos_file_mkdir,
		.remove = __xos_file_remove,
		.access = __xos_file_access,
		.walk = __xos_file_walk,
		.read = __xos_file_read,
		.write = __xos_file_write,
		.seek = __xos_file_seek,
		.tell = __xos_file_tell,
		.length = __xos_file_length,
		.sync = __xos_file_sync,
	},

	.coroutine = {
		.make = __xos_coroutine_make,
		.jump = __xos_coroutine_jump,
	},

	.thread = {
		.create = __xos_thread_create,
		.destroy = __xos_thread_destroy,
		.wait = __xos_thread_wait,
		.sleep = __xos_thread_sleep,
	},

	.mutex = {
		.init = __xos_mutex_init,
		.exit = __xos_mutex_exit,
		.lock = __xos_mutex_lock,
		.trylock = __xos_mutex_trylock,
		.unlock = __xos_mutex_unlock,
	},

	.semaphore = {
		.init = __xos_semaphore_init,
		.exit = __xos_semaphore_exit,
		.wait = __xos_semaphore_wait,
		.post = __xos_semaphore_post,
	},

	.other = {
		.strcpy = __xos_other_strcpy,
		.strncpy = __xos_other_strncpy,
		.strcat = __xos_other_strcat,
		.strncat = __xos_other_strncat,
		.strlen = __xos_other_strlen,
		.strnlen = __xos_other_strnlen,
		.strcmp = __xos_other_strcmp,
		.strncmp = __xos_other_strncmp,
		.strcasecmp = __xos_other_strcasecmp,
		.strncasecmp = __xos_other_strncasecmp,

		.memset = __xos_other_memset,
		.memcpy = __xos_other_memcpy,
		.memmove = __xos_other_memmove,
		.memchr = __xos_other_memchr,
		.memcmp = __xos_other_memcmp,
	},
};

void xos_environ_init(struct xos_environ_t * env)
{
	if(env)
	{
		/*
		 * malloc
		 */
		if(env->mem.malloc)
			__xos_environ.mem.malloc = env->mem.malloc;
		if(env->mem.memalign)
			__xos_environ.mem.memalign = env->mem.memalign;
		if(env->mem.realloc)
			__xos_environ.mem.realloc = env->mem.realloc;
		if(env->mem.calloc)
			__xos_environ.mem.calloc = env->mem.calloc;
		if(env->mem.free)
			__xos_environ.mem.free = env->mem.free;
		if(env->mem.meminfo)
			__xos_environ.mem.meminfo = env->mem.meminfo;

		/*
		 * dma
		 */
		if(env->dma.alloc_coherent)
			__xos_environ.dma.alloc_coherent = env->dma.alloc_coherent;
		if(env->dma.free_coherent)
			__xos_environ.dma.free_coherent = env->dma.free_coherent;
		if(env->dma.alloc_noncoherent)
			__xos_environ.dma.alloc_noncoherent = env->dma.alloc_noncoherent;
		if(env->dma.free_noncoherent)
			__xos_environ.dma.free_noncoherent = env->dma.free_noncoherent;
		if(env->dma.sync)
			__xos_environ.dma.sync = env->dma.sync;

		/*
		 * io
		 */
		if(env->io.read8)
			__xos_environ.io.read8 = env->io.read8;
		if(env->io.write8)
			__xos_environ.io.write8 = env->io.write8;
		if(env->io.read16)
			__xos_environ.io.read16 = env->io.read16;
		if(env->io.write16)
			__xos_environ.io.write16 = env->io.write16;
		if(env->io.read32)
			__xos_environ.io.read32 = env->io.read32;
		if(env->io.write32)
			__xos_environ.io.write32 = env->io.write32;
		if(env->io.read64)
			__xos_environ.io.read64 = env->io.read64;
		if(env->io.write64)
			__xos_environ.io.write64 = env->io.write64;

		/*
		 * stdio
		 */
		if(env->stdio.read)
			__xos_environ.stdio.read = env->stdio.read;
		if(env->stdio.write)
			__xos_environ.stdio.write = env->stdio.write;

		/*
		 * pm
		 */
		if(env->pm.shutdown)
			__xos_environ.pm.shutdown = env->pm.shutdown;
		if(env->pm.reboot)
			__xos_environ.pm.reboot = env->pm.reboot;
		if(env->pm.standby)
			__xos_environ.pm.standby = env->pm.standby;

		/*
		 * copyright
		 */
		if(env->copyright.uniqueid)
			__xos_environ.copyright.uniqueid = env->copyright.uniqueid;
		if(env->copyright.verify)
			__xos_environ.copyright.verify = env->copyright.verify;

		/*
		 * file
		 */
		if(env->file.cwd)
			__xos_environ.file.cwd = env->file.cwd;
		if(env->file.open)
			__xos_environ.file.open = env->file.open;
		if(env->file.close)
			__xos_environ.file.close = env->file.close;
		if(env->file.isdir)
			__xos_environ.file.isdir = env->file.isdir;
		if(env->file.isfile)
			__xos_environ.file.isfile = env->file.isfile;
		if(env->file.mode)
			__xos_environ.file.mode = env->file.mode;
		if(env->file.mkdir)
			__xos_environ.file.mkdir = env->file.mkdir;
		if(env->file.remove)
			__xos_environ.file.remove = env->file.remove;
		if(env->file.access)
			__xos_environ.file.access = env->file.access;
		if(env->file.walk)
			__xos_environ.file.walk = env->file.walk;
		if(env->file.read)
			__xos_environ.file.read = env->file.read;
		if(env->file.write)
			__xos_environ.file.write = env->file.write;
		if(env->file.seek)
			__xos_environ.file.seek = env->file.seek;
		if(env->file.tell)
			__xos_environ.file.tell = env->file.tell;
		if(env->file.length)
			__xos_environ.file.length = env->file.length;
		if(env->file.sync)
			__xos_environ.file.sync = env->file.sync;

		/*
		 * coroutine
		 */
		if(env->coroutine.make)
			__xos_environ.coroutine.make = env->coroutine.make;
		if(env->coroutine.jump)
			__xos_environ.coroutine.jump = env->coroutine.jump;

		/*
		 * thread
		 */
		if(env->thread.create)
			__xos_environ.thread.create = env->thread.create;
		if(env->thread.destroy)
			__xos_environ.thread.destroy = env->thread.destroy;
		if(env->thread.wait)
			__xos_environ.thread.wait = env->thread.wait;
		if(env->thread.sleep)
			__xos_environ.thread.sleep = env->thread.sleep;

		/*
		 * mutex
		 */
		if(env->mutex.init)
			__xos_environ.mutex.init = env->mutex.init;
		if(env->mutex.exit)
			__xos_environ.mutex.exit = env->mutex.exit;
		if(env->mutex.lock)
			__xos_environ.mutex.lock = env->mutex.lock;
		if(env->mutex.trylock)
			__xos_environ.mutex.trylock = env->mutex.trylock;
		if(env->mutex.unlock)
			__xos_environ.mutex.unlock = env->mutex.unlock;

		/*
		 * semaphore
		 */
		if(env->semaphore.init)
			__xos_environ.semaphore.init = env->semaphore.init;
		if(env->semaphore.exit)
			__xos_environ.semaphore.exit = env->semaphore.exit;
		if(env->semaphore.wait)
			__xos_environ.semaphore.wait = env->semaphore.wait;
		if(env->semaphore.post)
			__xos_environ.semaphore.post = env->semaphore.post;

		/*
		 * other
		 */
		if(env->other.strcpy)
			__xos_environ.other.strcpy = env->other.strcpy;
		if(env->other.strncpy)
			__xos_environ.other.strncpy = env->other.strncpy;
		if(env->other.strcat)
			__xos_environ.other.strcat = env->other.strcat;
		if(env->other.strncat)
			__xos_environ.other.strncat = env->other.strncat;
		if(env->other.strlen)
			__xos_environ.other.strlen = env->other.strlen;
		if(env->other.strnlen)
			__xos_environ.other.strnlen = env->other.strnlen;
		if(env->other.strcmp)
			__xos_environ.other.strcmp = env->other.strcmp;
		if(env->other.strncmp)
			__xos_environ.other.strncmp = env->other.strncmp;
		if(env->other.strcasecmp)
			__xos_environ.other.strcasecmp = env->other.strcasecmp;
		if(env->other.strncasecmp)
			__xos_environ.other.strncasecmp = env->other.strncasecmp;

		if(env->other.memset)
			__xos_environ.other.memset = env->other.memset;
		if(env->other.memcpy)
			__xos_environ.other.memcpy = env->other.memcpy;
		if(env->other.memmove)
			__xos_environ.other.memmove = env->other.memmove;
		if(env->other.memchr)
			__xos_environ.other.memchr = env->other.memchr;
		if(env->other.memcmp)
			__xos_environ.other.memcmp = env->other.memcmp;
	}
}
