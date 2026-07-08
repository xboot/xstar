#ifndef __FREERTOS_H__
#define __FREERTOS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>
#include <cache.h>

void freertos_init(void);
void freertos_run(void (*func)(void *));
void freertos_exit(void);

char * freertos_copyright_uniqueid(void);
int freertos_copyright_verify(void);

void * freertos_dma_alloc_coherent(unsigned long size);
void freertos_dma_free_coherent(void * addr);
void * freertos_dma_alloc_noncoherent(unsigned long size);
void freertos_dma_free_noncoherent(void * addr);
void freertos_dma_sync(void * addr, unsigned long size, int flag);

uint8_t freertos_io_read8(io_addr_t addr);
void freertos_io_write8(io_addr_t addr, uint8_t value);
uint16_t freertos_io_read16(io_addr_t addr);
void freertos_io_write16(io_addr_t addr, uint16_t value);
uint32_t freertos_io_read32(io_addr_t addr);
void freertos_io_write32(io_addr_t addr, uint32_t value);
uint64_t freertos_io_read64(io_addr_t addr);
void freertos_io_write64(io_addr_t addr, uint64_t value);

void * freertos_mem_malloc(size_t size);
void * freertos_mem_memalign(size_t align, size_t size);
void * freertos_mem_realloc(void * ptr, size_t size);
void * freertos_mem_calloc(size_t nmemb, size_t size);
void freertos_mem_free(void * ptr);
void freertos_mem_meminfo(size_t * mused, size_t * mfree);

void freertos_mutex_init(struct mutex_t * lock);
void freertos_mutex_exit(struct mutex_t * lock);
int freertos_mutex_lock(struct mutex_t * lock);
int freertos_mutex_trylock(struct mutex_t * lock);
int freertos_mutex_unlock(struct mutex_t * lock);

void freertos_pm_shutdown(void);
void freertos_pm_reboot(void);
void freertos_pm_standby(void);

void freertos_semaphore_init(struct semaphore_t * sem, uint32_t count);
void freertos_semaphore_exit(struct semaphore_t * sem);
int freertos_semaphore_wait(struct semaphore_t * sem, uint32_t timeout);
int freertos_semaphore_post(struct semaphore_t * sem);

struct thread_t * freertos_thread_create(const char * name, void (*func)(void *), void * data, int stksz);
void freertos_thread_destroy(struct thread_t * thread);
void freertos_thread_wait(struct thread_t * thread);
void freertos_thread_sleep(uint64_t ns);

void freertos_timer_init(void);
void freertos_timer_exit(void);
void freertos_timer_next(uint64_t time, void (*cb)(void *), void * data);
uint64_t freertos_timer_count(void);
uint64_t freertos_timer_frequency(void);

void * riscv32_coroutine_make(void * stack, size_t size, void (*func)(struct co_transfer_t));
struct co_transfer_t riscv32_coroutine_jump(void * fctx, void * priv);

#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_H__ */
