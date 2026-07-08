#include <xstar.h>
#include <baremetal/baremetal.h>

static struct xos_environ_t env = {
	.mem = {
		.malloc = baremetal_malloc,
		.memalign = baremetal_memalign,
		.realloc = baremetal_realloc,
		.calloc = baremetal_calloc,
		.free = baremetal_free,
		.meminfo = baremetal_meminfo,
	},

	.dma = {
		.alloc_coherent = baremetal_dma_alloc_coherent,
		.free_coherent = baremetal_dma_free_coherent,
		.alloc_noncoherent = baremetal_dma_alloc_noncoherent,
		.free_noncoherent = baremetal_dma_free_noncoherent,
		.sync = baremetal_dma_sync,
	},

	.io = {
		.read8 = baremetal_io_read8,
		.write8 = baremetal_io_write8,
		.read16 = baremetal_io_read16,
		.write16 = baremetal_io_write16,
		.read32 = baremetal_io_read32,
		.write32 = baremetal_io_write32,
		.read64 = baremetal_io_read64,
		.write64 = baremetal_io_write64,
	},

	.stdio = {
		.read = NULL,
		.write = NULL,
	},

	.pm = {
		.shutdown = baremetal_pm_shutdown,
		.reboot = baremetal_pm_reboot,
		.standby = baremetal_pm_standby,
	},

	.copyright = {
		.uniqueid = baremetal_copyright_uniqueid,
		.verify = baremetal_copyright_verify,
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
		.make = arm64_coroutine_make,
		.jump = arm64_coroutine_jump,
	},

	.thread = {
		.create = NULL,
		.destroy = NULL,
		.wait = NULL,
		.sleep = NULL,
	},

	.mutex = {
		.init = NULL,
		.exit = NULL,
		.lock = NULL,
		.trylock = NULL,
		.unlock = NULL,
	},

	.semaphore = {
		.init = NULL,
		.exit = NULL,
		.wait = NULL,
		.post = NULL,
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

		.memset = NULL,
		.memcpy = NULL,
		.memmove = NULL,
		.memchr = NULL,
		.memcmp = NULL,
	},
};

void xstar_main(void)
{
	xstar_init(&env, NULL);
	shell_system("shell;");
	xstar_exit();
}
