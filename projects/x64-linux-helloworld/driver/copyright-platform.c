#include <xstar.h>

static const char * platform_uniqueid(void)
{
	return NULL;
}

static int platform_keygen(const char * msg, void * key, int maxlen)
{
	return 0;
}

static int platform_verify(void)
{
	return 1;
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
