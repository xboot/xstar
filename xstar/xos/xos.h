#ifndef __XSTAR_XOS_H__
#define __XSTAR_XOS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>
#include <libx/aes128.h>
#include <libx/aes256.h>
#include <libx/atantab.h>
#include <libx/backoff.h>
#include <libx/base64.h>
#include <libx/bcd.h>
#include <libx/biquad.h>
#include <libx/breathing.h>
#include <libx/byteorder.h>
#include <libx/charset.h>
#include <libx/complex.h>
#include <libx/costab.h>
#include <libx/crc16.h>
#include <libx/crc32.h>
#include <libx/crc8.h>
#include <libx/db.h>
#include <libx/div.h>
#include <libx/ds.h>
#include <libx/dtree.h>
#include <libx/easing.h>
#include <libx/ecdsa256.h>
#include <libx/ewma.h>
#include <libx/fft.h>
#include <libx/fifo.h>
#include <libx/hexdump.h>
#include <libx/hmap.h>
#include <libx/initcall.h>
#include <libx/interleaver.h>
#include <libx/json.h>
#include <libx/kalman.h>
#include <libx/keyfilter.h>
#include <libx/kobj.h>
#include <libx/ktime.h>
#include <libx/list.h>
#include <libx/log2.h>
#include <libx/lru.h>
#include <libx/lsort.h>
#include <libx/mean.h>
#include <libx/median.h>
#include <libx/mm.h>
#include <libx/path.h>
#include <libx/ps.h>
#include <libx/qrcgen.h>
#include <libx/queue.h>
#include <libx/rbtree.h>
#include <libx/rc4.h>
#include <libx/rs.h>
#include <libx/sarg.h>
#include <libx/sha1.h>
#include <libx/sha256.h>
#include <libx/shash.h>
#include <libx/slist.h>
#include <libx/spring.h>
#include <libx/sqrti.h>
#include <libx/tsfilter.h>
#include <libx/unaligned.h>
#include <libx/uri.h>
#include <libx/uuid.h>
#include <libx/vmcrypt.h>
#include <libx/winfunc.h>
#include <libx/xdef.h>
#include <libx/yuv.h>

struct co_transfer_t {
	void * fctx;
	void * priv;
};

struct xos_environ_t {
	struct {
		void * (*malloc)(size_t size);
		void * (*memalign)(size_t align, size_t size);
		void * (*realloc)(void * ptr, size_t size);
		void * (*calloc)(size_t nmemb, size_t size);
		void (*free)(void * ptr);
		void (*meminfo)(size_t * mused, size_t * mfree);
	} mem;

	struct {
		void * (*alloc_coherent)(unsigned long size);
		void (*free_coherent)(void * addr);
		void * (*alloc_noncoherent)(unsigned long size);
		void (*free_noncoherent)(void * addr);
		void (*sync)(void * addr, unsigned long size, int flag);
	} dma;

	struct {
		uint8_t (*read8)(io_addr_t addr);
		void (*write8)(io_addr_t addr, uint8_t value);
		uint16_t (*read16)(io_addr_t addr);
		void (*write16)(io_addr_t addr, uint16_t value);
		uint32_t (*read32)(io_addr_t addr);
		void (*write32)(io_addr_t addr, uint32_t value);
		uint64_t (*read64)(io_addr_t addr);
		void (*write64)(io_addr_t addr, uint64_t value);
	} io;

	struct {
		ssize_t (*read)(void * buf, size_t count);
		ssize_t (*write)(void * buf, size_t count);
	} stdio;

	struct {
		void (*shutdown)(void);
		void (*reboot)(void);
		void (*standby)(void);
	} pm;

	struct {
		char * (*uniqueid)(void);
		int (*verify)(void);
	} copyright;

	struct {
		char * (*cwd)(void);
		int (*open)(const char * path, const char * mode);
		int (*close)(int fd);
		int (*isdir)(const char * path);
		int (*isfile)(const char * path);
		int (*mode)(const char * path);
		int (*mkdir)(const char * path);
		int (*remove)(const char * path);
		int (*access)(const char * path, const char * mode);
		void (*walk)(const char * path, void (*cb)(const char * dir, const char * name, void * data), const char * dir, void * data);
		ssize_t (*read)(int fd, void * buf, size_t count);
		ssize_t (*write)(int fd, const void * buf, size_t count);
		int64_t (*seek)(int fd, int64_t offset);
		int64_t (*tell)(int fd);
		int64_t (*length)(int fd);
		void (*sync)(int fd);
	} file;

	struct {
		void * (*make)(void * stack, size_t size, void (*func)(struct co_transfer_t));
		struct co_transfer_t (*jump)(void * fctx, void * priv);
	} coroutine;

	struct {
		struct thread_t * (*create)(const char * name, void (*func)(void *), void * data, int stksz);
		void (*destroy)(struct thread_t * thread);
		void (*wait)(struct thread_t * thread);
		void (*sleep)(uint64_t ns);
	} thread;

	struct {
		void (*init)(struct mutex_t * lock);
		void (*exit)(struct mutex_t * lock);
		int (*lock)(struct mutex_t * lock);
		int (*trylock)(struct mutex_t * lock);
		int (*unlock)(struct mutex_t * lock);
	} mutex;

	struct {
		void (*init)(struct semaphore_t * sem, uint32_t count);
		void (*exit)(struct semaphore_t * sem);
		int (*wait)(struct semaphore_t * sem, uint32_t timeout);
		int (*post)(struct semaphore_t * sem);
	} semaphore;

	struct {
		char * (*strcpy)(char * dest, const char * src);
		char * (*strncpy)(char * dest, const char * src, size_t n);
		char * (*strcat)(char * dest, const char * src);
		char * (*strncat)(char * dest, const char * src, size_t n);
		size_t (*strlen)(const char * s);
		size_t (*strnlen)(const char * s, size_t n);
		int (*strcmp)(const char * s1, const char * s2);
		int (*strncmp)(const char * s1, const char * s2, size_t n);
		int (*strcasecmp)(const char * s1, const char * s2);
		int (*strncasecmp)(const char * s1, const char * s2, size_t n);

		void * (*memset)(void * s, int c, size_t n);
		void * (*memcpy)(void * dest, const void * src, size_t len);
		void * (*memmove)(void * dest, const void * src, size_t n);
		void * (*memchr)(const void * s, int c, size_t n);
		int (*memcmp)(const void * s1, const void * s2, size_t n);
	} other;
};
extern struct xos_environ_t __xos_environ;

/*
 * malloc
 */
static inline void * xos_mem_malloc(size_t size)
{
	#undef malloc
	return __xos_environ.mem.malloc(size);
}

static inline void * xos_mem_memalign(size_t align, size_t size)
{
	#undef memalign
	return __xos_environ.mem.memalign(align, size);
}

static inline void * xos_mem_realloc(void * ptr, size_t size)
{
	#undef realloc
	return __xos_environ.mem.realloc(ptr, size);
}

static inline void * xos_mem_calloc(size_t nmemb, size_t size)
{
	#undef calloc
	return __xos_environ.mem.calloc(nmemb, size);
}

static inline void xos_mem_free(void * ptr)
{
	#undef free
	__xos_environ.mem.free(ptr);
}

static inline void xos_mem_meminfo(size_t * mused, size_t * mfree)
{
	#undef meminfo
	__xos_environ.mem.meminfo(mused, mfree);
}

/*
 * dma
 */
static inline void * xos_dma_alloc_coherent(unsigned long size)
{
	#undef alloc_coherent
	return __xos_environ.dma.alloc_coherent(size);
}

static inline void xos_dma_free_coherent(void * addr)
{
	#undef free_coherent
	__xos_environ.dma.free_coherent(addr);
}

static inline void * xos_dma_alloc_noncoherent(unsigned long size)
{
	#undef alloc_noncoherent
	return __xos_environ.dma.alloc_noncoherent(size);
}

static inline void xos_dma_free_noncoherent(void * addr)
{
	#undef free_noncoherent
	__xos_environ.dma.free_noncoherent(addr);
}

static inline void xos_dma_sync(void * addr, unsigned long size, int flag)
{
	#undef sync
	__xos_environ.dma.sync(addr, size, flag);
}

/*
 * io
 */
static inline uint8_t xos_io_read8(io_addr_t addr)
{
	#undef read8
	return __xos_environ.io.read8(addr);
}

static inline void xos_io_write8(io_addr_t addr, uint8_t value)
{
	#undef write8
	__xos_environ.io.write8(addr, value);
}

static inline uint16_t xos_io_read16(io_addr_t addr)
{
	#undef read16
	return __xos_environ.io.read16(addr);
}

static inline void xos_io_write16(io_addr_t addr, uint16_t value)
{
	#undef write16
	__xos_environ.io.write16(addr, value);
}

static inline uint32_t xos_io_read32(io_addr_t addr)
{
	#undef read32
	return __xos_environ.io.read32(addr);
}

static inline void xos_io_write32(io_addr_t addr, uint32_t value)
{
	#undef write32
	__xos_environ.io.write32(addr, value);
}

static inline uint64_t xos_io_read64(io_addr_t addr)
{
	#undef read64
	return __xos_environ.io.read64(addr);
}

static inline void xos_io_write64(io_addr_t addr, uint64_t value)
{
	#undef write64
	__xos_environ.io.write64(addr, value);
}

static inline void xos_io_clrbits8(io_addr_t addr, uint8_t clr)
{
	xos_io_write8(addr, xos_io_read8(addr) & ~clr);
}

static inline void xos_io_setbits8(io_addr_t addr, uint8_t set)
{
	xos_io_write8(addr, xos_io_read8(addr) | set);
}

static inline void xos_io_clrsetbits8(io_addr_t addr, uint8_t clr, uint8_t set)
{
	xos_io_write8(addr, (xos_io_read8(addr) & ~clr) | set);
}

static inline void xos_io_clrbits16(io_addr_t addr, uint16_t clr)
{
	xos_io_write16(addr, xos_io_read16(addr) & ~clr);
}

static inline void xos_io_setbits16(io_addr_t addr, uint16_t set)
{
	xos_io_write16(addr, xos_io_read16(addr) | set);
}

static inline void xos_io_clrsetbits16(io_addr_t addr, uint16_t clr, uint16_t set)
{
	xos_io_write16(addr, (xos_io_read16(addr) & ~clr) | set);
}

static inline void xos_io_clrbits32(io_addr_t addr, uint32_t clr)
{
	xos_io_write32(addr, xos_io_read32(addr) & ~clr);
}

static inline void xos_io_setbits32(io_addr_t addr, uint32_t set)
{
	xos_io_write32(addr, xos_io_read32(addr) | set);
}

static inline void xos_io_clrsetbits32(io_addr_t addr, uint32_t clr, uint32_t set)
{
	xos_io_write32(addr, (xos_io_read32(addr) & ~clr) | set);
}

static inline void xos_io_clrbits64(io_addr_t addr, uint64_t clr)
{
	xos_io_write64(addr, xos_io_read64(addr) & ~clr);
}

static inline void xos_io_setbits64(io_addr_t addr, uint64_t set)
{
	xos_io_write64(addr, xos_io_read64(addr) | set);
}

static inline void xos_io_clrsetbits64(io_addr_t addr, uint64_t clr, uint64_t set)
{
	xos_io_write64(addr, (xos_io_read64(addr) & ~clr) | set);
}

/*
 * stdio
 */
static inline ssize_t xos_stdio_read(void * buf, size_t count)
{
	#undef read
	return __xos_environ.stdio.read(buf, count);
}

static inline ssize_t xos_stdio_write(void * buf, size_t count)
{
	#undef write
	return __xos_environ.stdio.write(buf, count);
}

/*
 * pm
 */
static inline void xos_pm_shutdown(void)
{
	#undef shutdown
	__xos_environ.pm.shutdown();
}

static inline void xos_pm_reboot(void)
{
	#undef reboot
	__xos_environ.pm.reboot();
}

static inline void xos_pm_standby(void)
{
	#undef standby
	__xos_environ.pm.standby();
}

/*
 * copyright
 */
static inline char * xos_copyright_uniqueid(void)
{
	#undef uniqueid
	return __xos_environ.copyright.uniqueid();
}

static inline int xos_copyright_verify(void)
{
	#undef verify
	return __xos_environ.copyright.verify();
}

/*
 * file
 */
static inline char * xos_file_cwd(void)
{
	#undef cwd
	return __xos_environ.file.cwd();
}

static inline int xos_file_open(const char * path, const char * mode)
{
	#undef open
	return __xos_environ.file.open(path, mode);
}

static inline int xos_file_close(int fd)
{
	#undef close
	return __xos_environ.file.close(fd);
}

static inline int xos_file_isdir(const char * path)
{
	#undef isdir
	return __xos_environ.file.isdir(path);
}

static inline int xos_file_isfile(const char * path)
{
	#undef isfile
	return __xos_environ.file.isfile(path);
}

static inline int xos_file_mode(const char * path)
{
	#undef mode
	return __xos_environ.file.mode(path);
}

static inline int xos_file_mkdir(const char * path)
{
	#undef mkdir
	return __xos_environ.file.mkdir(path);
}

static inline int xos_file_remove(const char * path)
{
	#undef remove
	return __xos_environ.file.remove(path);
}

static inline int xos_file_access(const char * path, const char * mode)
{
	#undef access
	return __xos_environ.file.access(path, mode);
}

static inline void xos_file_walk(const char * path, void (*cb)(const char * dir, const char * name, void * data), const char * dir, void * data)
{
	#undef walk
	__xos_environ.file.walk(path, cb, dir, data);
}

static inline ssize_t xos_file_read(int fd, void * buf, size_t count)
{
	#undef read
	return __xos_environ.file.read(fd, buf, count);
}

static inline ssize_t xos_file_write(int fd, const void * buf, size_t count)
{
	#undef write
	return __xos_environ.file.write(fd, buf, count);
}

static inline int64_t xos_file_seek(int fd, int64_t offset)
{
	#undef seek
	return __xos_environ.file.seek(fd, offset);
}

static inline int64_t xos_file_tell(int fd)
{
	#undef tell
	return __xos_environ.file.tell(fd);
}

static inline int64_t xos_file_length(int fd)
{
	#undef length
	return __xos_environ.file.length(fd);
}

static inline void xos_file_sync(int fd)
{
	#undef sync
	__xos_environ.file.sync(fd);
}

/*
 * coroutine
 */
static inline void * xos_coroutine_make(void * stack, size_t size, void (*func)(struct co_transfer_t))
{
	#undef make
	return __xos_environ.coroutine.make(stack, size, func);
}

static inline struct co_transfer_t xos_coroutine_jump(void * fctx, void * priv)
{
	#undef jump
	return __xos_environ.coroutine.jump(fctx, priv);
}

/*
 * thread
 */
static inline struct thread_t * xos_thread_create(const char * name, void (*func)(void *), void * data, int stksz)
{
	#undef create
	return __xos_environ.thread.create(name, func, data, stksz);
}

static inline void xos_thread_destroy(struct thread_t * thread)
{
	#undef destroy
	__xos_environ.thread.destroy(thread);
}

static inline void xos_thread_wait(struct thread_t * thread)
{
	#undef wait
	__xos_environ.thread.wait(thread);
}

static inline void xos_thread_sleep(uint64_t ns)
{
	#undef sleep
	__xos_environ.thread.sleep(ns);
}

static inline void xos_thread_nsleep(uint64_t ns)
{
	xos_thread_sleep(ns);
}

static inline void xos_thread_usleep(uint64_t us)
{
	xos_thread_sleep(us * 1000);
}

static inline void xos_thread_msleep(uint64_t ms)
{
	xos_thread_sleep(ms * 1000000);
}

/*
 * mutex
 */
static inline void xos_mutex_init(struct mutex_t * lock)
{
	#undef init
	__xos_environ.mutex.init(lock);
}

static inline void xos_mutex_exit(struct mutex_t * lock)
{
	#undef exit
	__xos_environ.mutex.exit(lock);
}

static inline int xos_mutex_lock(struct mutex_t * lock)
{
	#undef lock
	return __xos_environ.mutex.lock(lock);
}

static inline int xos_mutex_trylock(struct mutex_t * lock)
{
	#undef trylock
	return __xos_environ.mutex.trylock(lock);
}

static inline int xos_mutex_unlock(struct mutex_t * lock)
{
	#undef unlock
	return __xos_environ.mutex.unlock(lock);
}

/*
 * semaphore
 */
static inline void xos_semaphore_init(struct semaphore_t * sem, uint32_t count)
{
	#undef init
	__xos_environ.semaphore.init(sem, count);
}

static inline void xos_semaphore_exit(struct semaphore_t * sem)
{
	#undef exit
	__xos_environ.semaphore.exit(sem);
}

static inline int xos_semaphore_wait(struct semaphore_t * sem, uint32_t timeout)
{
	#undef wait
	return __xos_environ.semaphore.wait(sem, timeout);
}

static inline int xos_semaphore_post(struct semaphore_t * sem)
{
	#undef post
	return __xos_environ.semaphore.post(sem);
}

/*
 * other
 */
static inline char * xos_strcpy(char * dest, const char * src)
{
	#undef strcpy
	return __xos_environ.other.strcpy(dest, src);
}

static inline char * xos_strncpy(char * dest, const char * src, size_t n)
{
	#undef strncpy
	return __xos_environ.other.strncpy(dest, src, n);
}

static inline char * xos_strcat(char * dest, const char * src)
{
	#undef strcat
	return __xos_environ.other.strcat(dest, src);
}

static inline char * xos_strncat(char * dest, const char * src, size_t n)
{
	#undef strncat
	return __xos_environ.other.strncat(dest, src, n);
}

static inline size_t xos_strlen(const char * s)
{
	#undef strlen
	return __xos_environ.other.strlen(s);
}

static inline size_t xos_strnlen(const char * s, size_t n)
{
	#undef strnlen
	return __xos_environ.other.strnlen(s, n);
}

static inline int xos_strcmp(const char * s1, const char * s2)
{
	#undef strcmp
	return __xos_environ.other.strcmp(s1, s2);
}

static inline int xos_strncmp(const char * s1, const char * s2, size_t n)
{
	#undef strncmp
	return __xos_environ.other.strncmp(s1, s2, n);
}

static inline int xos_strcasecmp(const char * s1, const char * s2)
{
	#undef strcasecmp
	return __xos_environ.other.strcasecmp(s1, s2);
}

static inline int xos_strncasecmp(const char * s1, const char * s2, size_t n)
{
	#undef strncasecmp
	return __xos_environ.other.strncasecmp(s1, s2, n);
}

static inline void * xos_memset(void * s, int c, size_t n)
{
	#undef memset
	return __xos_environ.other.memset(s, c, n);
}

static inline void * xos_memcpy(void * dest, const void * src, size_t len)
{
	#undef memcpy
	return __xos_environ.other.memcpy(dest, src, len);
}

static inline void * xos_memmove(void * dest, const void * src, size_t n)
{
	#undef memmove
	return __xos_environ.other.memmove(dest, src, n);
}

static inline void * xos_memchr(const void * s, int c, size_t n)
{
	#undef memchr
	return __xos_environ.other.memchr(s, c, n);
}

static inline int xos_memcmp(const void * s1, const void * s2, size_t n)
{
	#undef memcmp
	return __xos_environ.other.memcmp(s1, s2, n);
}

/*
 * environ
 */
void xos_environ_init(struct xos_environ_t * env);

/*
 * others
 */
int xos_vsnprintf(char * buf, size_t n, const char * fmt, va_list ap);
int xos_vasprintf(char ** s, const char * fmt, va_list ap);
int xos_vprintf(const char * fmt, va_list ap);
int xos_vsscanf(const char * buf, const char * fmt, va_list ap);
int xos_snprintf(char * buf, size_t n, const char * fmt, ...);
int xos_asprintf(char ** s, const char * fmt, ...);
int xos_sprintf(char * buf, const char * fmt, ...);
int xos_sscanf(const char * buf, const char * fmt, ...);
int xos_printf(const char * fmt, ...);
int xos_getchar(void);
int xos_putchar(int c);

int xos_isalnum(int c);
int xos_isalpha(int c);
int xos_isascii(int c);
int xos_isblank(int c);
int xos_iscntrl(int c);
int xos_isdigit(int c);
int xos_isgraph(int c);
int xos_islower(int c);
int xos_isprint(int c);
int xos_ispunct(int c);
int xos_isspace(int c);
int xos_isupper(int c);
int xos_isxdigit(int c);
int xos_toascii(int c);
int xos_tolower(int c);
int xos_toupper(int c);

ssize_t xos_strscpy(char * dest, const char * src, size_t n);
size_t xos_strlcat(char * dest, const char * src, size_t n);
size_t xos_strlcpy(char * dest, const char * src, size_t n);
size_t xos_strcspn(const char * s, const char * reject);
size_t xos_strspn(const char * s, const char * accept);
char * xos_strchr(const char * s, int c);
char * xos_strrchr(const char * s, int c);
char * xos_strnchr(const char * s, size_t n, int c);
char * xos_strstr(const char * s1, const char * s2);
char * xos_strnstr(const char * s1, const char * s2, size_t n);
char * xos_strpbrk(const char * s1, const char * s2);
char * xos_strsep(char ** s, const char * ct);
char * xos_strdup(const char * s);
char * xos_strndup(const char * s, size_t n);
char * xos_strtok(char * s, const char * sep, char ** p);
char * xos_ssize(char * buf, double size);
char * xos_strim(char * s);

int xos_strnatcmp(const char * s1, const char * s2);
int xos_strnatcasecmp(const char * s1, const char * s2);

int xos_abs(int n);
long xos_labs(long n);
long long xos_llabs(long long n);

int xos_atoi(const char * nptr);
long xos_atol(const char * nptr);
long long xos_atoll(const char * nptr);
double xos_atof(const char * nptr);

long xos_strtol(const char * nptr, char ** endptr, int base);
long long xos_strtoll(const char * nptr, char ** endptr, int base);
unsigned long xos_strtoul(const char * nptr, char ** endptr, int base);
unsigned long long xos_strtoull(const char * nptr, char ** endptr, int base);
double xos_strtod(const char * nptr, char ** endptr);

intmax_t xos_strtoimax(const char * nptr, char ** endptr, int base);
uintmax_t xos_strtoumax(const char * nptr, char ** endptr, int base);
intmax_t xos_strntoimax(const char * nptr, char ** endptr, int base, size_t n);
uintmax_t xos_strntoumax(const char * nptr, char ** endptr, int base, size_t n);

void * xos_bsearch(const void * key, const void * base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *));
void xos_qsort(void * aa, size_t n, size_t es, int (*cmp)(const void *, const void *));

#define XOS_RAND_MAX	(0x7fffffff)
void xos_srand(unsigned int seed);
int xos_rand(void);
int xos_random_int(int lower, int upper);
float xos_random_float(float lower, float upper);
double xos_random_double(double lower, double upper);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_XOS_H__ */
