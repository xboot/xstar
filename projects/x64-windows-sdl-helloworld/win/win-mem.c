#include <win/win.h>

void * win_mem_malloc(size_t size)
{
	return SDL_malloc(size);
}

void * win_mem_memalign(size_t align, size_t size)
{
	return SDL_malloc(size);
}

void * win_mem_realloc(void * ptr, size_t size)
{
	return SDL_realloc(ptr, size);
}

void * win_mem_calloc(size_t nmemb, size_t size)
{
	return SDL_calloc(nmemb, size);
}

void win_mem_free(void * ptr)
{
	SDL_free(ptr);
}

void win_mem_meminfo(size_t * mused, size_t * mfree)
{
	MEMORYSTATUSEX mi;
	mi.dwLength = sizeof(mi);
	GlobalMemoryStatusEx(&mi);

	if(mused)
		*mused = mi.ullTotalPhys - mi.ullAvailPhys;
	if(mfree)
		*mfree = mi.ullAvailPhys;
}
