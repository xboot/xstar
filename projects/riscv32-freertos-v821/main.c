#include <xstar.h>
#include <freertos/freertos.h>

extern void * sys_memset(void * s, int c, size_t n);
extern void * sys_memcpy(void * dest, const void * src, size_t len);
extern void * sys_memmove(void * dest, const void * src, size_t n);

static struct xos_environ_t env = {
	.mem = {
		.malloc = freertos_mem_malloc,
		.memalign = freertos_mem_memalign,
		.realloc = freertos_mem_realloc,
		.calloc = freertos_mem_calloc,
		.free = freertos_mem_free,
		.meminfo = freertos_mem_meminfo,
	},

	.dma = {
		.alloc_coherent = freertos_dma_alloc_coherent,
		.free_coherent = freertos_dma_free_coherent,
		.alloc_noncoherent = freertos_dma_alloc_noncoherent,
		.free_noncoherent = freertos_dma_free_noncoherent,
		.sync = freertos_dma_sync,
	},

	.io = {
		.read8 = freertos_io_read8,
		.write8 = freertos_io_write8,
		.read16 = freertos_io_read16,
		.write16 = freertos_io_write16,
		.read32 = freertos_io_read32,
		.write32 = freertos_io_write32,
		.read64 = freertos_io_read64,
		.write64 = freertos_io_write64,
	},

	.stdio = {
		.read = NULL,
		.write = NULL,
	},

	.pm = {
		.shutdown = freertos_pm_shutdown,
		.reboot = freertos_pm_reboot,
		.standby = freertos_pm_standby,
	},

	.copyright = {
		.uniqueid = freertos_copyright_uniqueid,
		.verify = freertos_copyright_verify,
	},

	.file = {
		.cwd = NULL,
		.open = NULL,
		.close = NULL,
		.isdir = NULL,
		.isfile = NULL,
		.mode = NULL,
		.mkdir = NULL,
		.remove = NULL,
		.access = NULL,
		.walk = NULL,
		.read = NULL,
		.write = NULL,
		.seek = NULL,
		.tell = NULL,
		.length = NULL,
		.sync = NULL,
	},

	.coroutine = {
		.make = riscv32_coroutine_make,
		.jump = riscv32_coroutine_jump,
	},

	.thread = {
		.create = freertos_thread_create,
		.destroy = freertos_thread_destroy,
		.wait = freertos_thread_wait,
		.sleep = freertos_thread_sleep,
	},

	.mutex = {
		.init = freertos_mutex_init,
		.exit = freertos_mutex_exit,
		.lock = freertos_mutex_lock,
		.trylock = freertos_mutex_trylock,
		.unlock = freertos_mutex_unlock,
	},

	.semaphore = {
		.init = freertos_semaphore_init,
		.exit = freertos_semaphore_exit,
		.wait = freertos_semaphore_wait,
		.post = freertos_semaphore_post,
	},

	.other = {
		.strcpy = NULL,
		.strncpy = NULL,
		.strcat = NULL,
		.strncat = NULL,
		.strlen = NULL,
		.strnlen = NULL,
		.strcmp = NULL,
		.strncmp = NULL,
		.strcasecmp = NULL,
		.strncasecmp = NULL,

		.memset = sys_memset,
		.memcpy = sys_memcpy,
		.memmove = sys_memmove,
		.memchr = NULL,
		.memcmp = NULL,
	},
};

static void task_entry(void * data)
{
	shell_system("shell;");
}

void xstar_main(void)
{
	freertos_init();
	{
		xstar_init(&env, NULL);
		freertos_run(task_entry);
		xstar_exit();
	}
	freertos_exit();
}
