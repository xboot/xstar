#include <xstar.h>
#include <win/win.h>

static struct xos_environ_t env = {
	.mem = {
		.malloc = win_mem_malloc,
		.memalign = win_mem_memalign,
		.realloc = win_mem_realloc,
		.calloc = win_mem_calloc,
		.free = win_mem_free,
		.meminfo = win_mem_meminfo,
	},

	.dma = {
		.alloc_coherent = win_dma_alloc_coherent,
		.free_coherent = win_dma_free_coherent,
		.alloc_noncoherent = win_dma_alloc_noncoherent,
		.free_noncoherent = win_dma_free_noncoherent,
		.sync = win_dma_sync,
	},

	.io = {
		.read8 = win_io_read8,
		.write8 = win_io_write8,
		.read16 = win_io_read16,
		.write16 = win_io_write16,
		.read32 = win_io_read32,
		.write32 = win_io_write32,
		.read64 = win_io_read64,
		.write64 = win_io_write64,
	},

	.stdio = {
		.read = win_stdio_read,
		.write = win_stdio_write,
	},

	.pm = {
		.shutdown = win_pm_shutdown,
		.reboot = win_pm_reboot,
		.standby = win_pm_standby,
	},

	.copyright = {
		.uniqueid = win_copyright_uniqueid,
		.verify = win_copyright_verify,
	},

	.file = {
		.cwd = win_file_cwd,
		.open = win_file_open,
		.close = win_file_close,
		.isdir = win_file_isdir,
		.isfile = win_file_isfile,
		.mode = win_file_mode,
		.mkdir = win_file_mkdir,
		.remove = win_file_remove,
		.access = win_file_access,
		.walk = win_file_walk,
		.read = win_file_read,
		.write = win_file_write,
		.seek = win_file_seek,
		.tell = win_file_tell,
		.length = win_file_length,
		.sync = win_file_sync,
	},

	.coroutine = {
		.make = x64_coroutine_make,
		.jump = x64_coroutine_jump,
	},

	.thread = {
		.create = win_thread_create,
		.destroy = win_thread_destroy,
		.wait = win_thread_wait,
		.sleep = win_thread_sleep,
	},

	.mutex = {
		.init = win_mutex_init,
		.exit = win_mutex_exit,
		.lock = win_mutex_lock,
		.trylock = win_mutex_trylock,
		.unlock = win_mutex_unlock,
	},

	.semaphore = {
		.init = win_semaphore_init,
		.exit = win_semaphore_exit,
		.wait = win_semaphore_wait,
		.post = win_semaphore_post,
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
	win_init();
	{
		xstar_init(&env, NULL);
		shell_system("shell;");
		xstar_exit();
	}
	win_exit();

	return 0;
}
