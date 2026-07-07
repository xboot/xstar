#ifndef __XSTAR_DRIVER_NVMEM_H__
#define __XSTAR_DRIVER_NVMEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>
#include <kernel/time/delay.h>
#include <kernel/time/timer.h>

struct nvmem_t {
	char * name;

	int (*capacity)(struct nvmem_t * m);
	int (*read)(struct nvmem_t * m, void * buf, int offset, int count);
	int (*write)(struct nvmem_t * m, void * buf, int offset, int count);
	void * priv;
};

struct nvmem_t * search_nvmem(const char * name);
struct nvmem_t * search_first_nvmem(void);
struct device_t * register_nvmem(struct nvmem_t * m, struct driver_t * drv);
void unregister_nvmem(struct nvmem_t * m);

int nvmem_capacity(struct nvmem_t * m);
int nvmem_read(struct nvmem_t * m, void * buf, int offset, int count);
int nvmem_write(struct nvmem_t * m, void * buf, int offset, int count);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_NVMEM_H__ */
