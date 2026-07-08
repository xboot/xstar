#include <xstar.h>
#include <linux/linux.h>

static struct xos_environ_t env = {
	.mem = {
		.malloc = linux_mem_malloc,
		.memalign = linux_mem_memalign,
		.realloc = linux_mem_realloc,
		.calloc = linux_mem_calloc,
		.free = linux_mem_free,
		.meminfo = linux_mem_meminfo,
	},

	.dma = {
		.alloc_coherent = linux_dma_alloc_coherent,
		.free_coherent = linux_dma_free_coherent,
		.alloc_noncoherent = linux_dma_alloc_noncoherent,
		.free_noncoherent = linux_dma_free_noncoherent,
		.sync = linux_dma_sync,
	},

	.io = {
		.read8 = linux_io_read8,
		.write8 = linux_io_write8,
		.read16 = linux_io_read16,
		.write16 = linux_io_write16,
		.read32 = linux_io_read32,
		.write32 = linux_io_write32,
		.read64 = linux_io_read64,
		.write64 = linux_io_write64,
	},

	.stdio = {
		.read = linux_stdio_read,
		.write = linux_stdio_write,
	},

	.pm = {
		.shutdown = linux_pm_shutdown,
		.reboot = linux_pm_reboot,
		.standby = linux_pm_standby,
	},

	.copyright = {
		.uniqueid = linux_copyright_uniqueid,
		.verify = linux_copyright_verify,
	},

	.file = {
		.cwd = linux_file_cwd,
		.open = linux_file_open,
		.close = linux_file_close,
		.isdir = linux_file_isdir,
		.isfile = linux_file_isfile,
		.mode = linux_file_mode,
		.mkdir = linux_file_mkdir,
		.remove = linux_file_remove,
		.access = linux_file_access,
		.walk = linux_file_walk,
		.read = linux_file_read,
		.write = linux_file_write,
		.seek = linux_file_seek,
		.tell = linux_file_tell,
		.length = linux_file_length,
		.sync = linux_file_sync,
	},

	.coroutine = {
		.make = arm32_coroutine_make,
		.jump = arm32_coroutine_jump,
	},

	.thread = {
		.create = linux_thread_create,
		.destroy = linux_thread_destroy,
		.wait = linux_thread_wait,
		.sleep = linux_thread_sleep,
	},

	.mutex = {
		.init = linux_mutex_init,
		.exit = linux_mutex_exit,
		.lock = linux_mutex_lock,
		.trylock = linux_mutex_trylock,
		.unlock = linux_mutex_unlock,
	},

	.semaphore = {
		.init = linux_semaphore_init,
		.exit = linux_semaphore_exit,
		.wait = linux_semaphore_wait,
		.post = linux_semaphore_post,
	},

	.other = {
		.strcpy = strcpy,
		.strncpy = strncpy,
		.strcat = strcat,
		.strncat = strncat,
		.strlen = strlen,
		.strnlen = strnlen,
		.strcmp = strcmp,
		.strncmp = strncmp,
		.strcasecmp = strcasecmp,
		.strncasecmp = strncasecmp,

		.memset = memset,
		.memcpy = memcpy,
		.memmove = memmove,
		.memchr = memchr,
		.memcmp = memcmp,
	},
};

int main(int argc, char * argv[])
{
	linux_init();
	{
		xstar_init(&env, NULL);
		shell_system("shell;");
		xstar_exit();
	}
	linux_exit();

	return 0;
}
