#include <linux/linux.h>

void * linux_dma_alloc_coherent(unsigned long size)
{
	return memalign(4096, size);
}

void linux_dma_free_coherent(void * addr)
{
	free(addr);
}

void * linux_dma_alloc_noncoherent(unsigned long size)
{
	return memalign(4096, size);
}

void linux_dma_free_noncoherent(void * addr)
{
	free(addr);
}

void linux_dma_sync(void * addr, unsigned long size, int flag)
{
}
