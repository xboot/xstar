#include <baremetal/baremetal.h>

static const char * platform_uniqueid(void)
{
	static char uniqueid[32 + 1] = { 0 };
	io_addr_t addr = 0x43006200;
	uint32_t sid[4];

	sid[0] = baremetal_io_read32(addr + 0 * 4);
	sid[1] = baremetal_io_read32(addr + 1 * 4);
	sid[2] = baremetal_io_read32(addr + 2 * 4);
	sid[3] = baremetal_io_read32(addr + 3 * 4);
	xos_snprintf(uniqueid, sizeof(uniqueid), "%08x%08x%08x%08x",sid[0], sid[1], sid[2], sid[3]);
	return uniqueid;
}

static int platform_verify(void)
{
	return 1;
}

static int platform_keygen(const char * msg, void * key, int maxlen)
{
	return 0;
}

static struct copyright_t copyright_platform = {
	.uniqueid	= platform_uniqueid,
	.keygen		= platform_keygen,
	.verify		= platform_verify,
};

static void copyright_platform_init(void)
{
	register_copyright(&copyright_platform);
}
core_initcall(copyright_platform_init);
