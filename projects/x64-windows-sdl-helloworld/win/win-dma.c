#include <win/win.h>

void * win_dma_alloc_coherent(unsigned long size)
{
	return SDL_aligned_alloc(4096, size);
}

void win_dma_free_coherent(void * addr)
{
	SDL_aligned_free(addr);
}

void * win_dma_alloc_noncoherent(unsigned long size)
{
	return SDL_aligned_alloc(4096, size);
}

void win_dma_free_noncoherent(void * addr)
{
	SDL_aligned_free(addr);
}

void win_dma_sync(void * addr, unsigned long size, int flag)
{
}
