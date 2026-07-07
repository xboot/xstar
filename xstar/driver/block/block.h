#ifndef __XSTAR_DRIVER_BLOCK_H__
#define __XSTAR_DRIVER_BLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/device.h>
#include <driver/driver.h>

struct block_t {
	char * name;

	uint64_t (*capacity)(struct block_t * blk);
	uint64_t (*read)(struct block_t * blk, uint8_t * buf, uint64_t offset, uint64_t count);
	uint64_t (*write)(struct block_t * blk, uint8_t * buf, uint64_t offset, uint64_t count);
	void (*sync)(struct block_t * blk);

	void * priv;
};

static inline uint64_t block_available(struct block_t * blk, uint64_t offset, uint64_t length)
{
	uint64_t cap;

	if(blk)
	{
		cap = blk->capacity(blk);
		if(offset + length > cap)
			return cap - offset;
		return length;
	}
	return 0;
}

struct block_t * search_block(const char * name);
struct device_t * register_block(struct block_t * blk, struct driver_t * drv);
void unregister_block(struct block_t * blk);
struct device_t * register_sub_block(struct block_t * pblk, uint64_t offset, uint64_t length, const char * name);
void unregister_sub_block(struct block_t * pblk);

uint64_t block_capacity(struct block_t * blk);
uint64_t block_read(struct block_t * blk, uint8_t * buf, uint64_t offset, uint64_t count);
uint64_t block_write(struct block_t * blk, uint8_t * buf, uint64_t offset, uint64_t count);
void block_sync(struct block_t * blk);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_BLOCK_H__ */
