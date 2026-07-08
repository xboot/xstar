#include <linux/linux.h>

void * linux_mem_malloc(size_t size)
{
	return malloc(size);
}

void * linux_mem_memalign(size_t align, size_t size)
{
	return memalign(align, size);
}

void * linux_mem_realloc(void * ptr, size_t size)
{
	return realloc(ptr, size);
}

void * linux_mem_calloc(size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
}

void linux_mem_free(void * ptr)
{
	free(ptr);
}

void linux_mem_meminfo(size_t * mused, size_t * mfree)
{
	struct sysinfo info;

	if(sysinfo(&info) == 0)
	{
		if(mused)
			*mused = (info.totalram - info.freeram) * info.mem_unit;
		if(mfree)
			*mfree = info.freeram * info.mem_unit;
	}
}
