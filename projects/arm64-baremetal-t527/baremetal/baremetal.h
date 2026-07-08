#ifndef __BAREMETAL_H__
#define __BAREMETAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>

char * baremetal_copyright_uniqueid(void);
int baremetal_copyright_verify(void);

void * baremetal_dma_alloc_coherent(unsigned long size);
void baremetal_dma_free_coherent(void * addr);
void * baremetal_dma_alloc_noncoherent(unsigned long size);
void baremetal_dma_free_noncoherent(void * addr);
void baremetal_dma_sync(void * addr, unsigned long size, int flag);

uint8_t baremetal_io_read8(io_addr_t addr);
void baremetal_io_write8(io_addr_t addr, uint8_t value);
uint16_t baremetal_io_read16(io_addr_t addr);
void baremetal_io_write16(io_addr_t addr, uint16_t value);
uint32_t baremetal_io_read32(io_addr_t addr);
void baremetal_io_write32(io_addr_t addr, uint32_t value);
uint64_t baremetal_io_read64(io_addr_t addr);
void baremetal_io_write64(io_addr_t addr, uint64_t value);

void * baremetal_malloc(size_t size);
void * baremetal_memalign(size_t align, size_t size);
void * baremetal_realloc(void * ptr, size_t size);
void * baremetal_calloc(size_t nmemb, size_t size);
void baremetal_free(void * ptr);
void baremetal_meminfo(size_t * mused, size_t * mfree);

void baremetal_pm_shutdown(void);
void baremetal_pm_reboot(void);
void baremetal_pm_standby(void);

void * arm64_coroutine_make(void * stack, size_t size, void (*func)(struct co_transfer_t));
struct co_transfer_t arm64_coroutine_jump(void * fctx, void * priv);

#ifdef __cplusplus
}
#endif

#endif /* __BAREMETAL_H__ */
