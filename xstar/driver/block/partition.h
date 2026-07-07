#ifndef __XSTAR_DRIVER_BLOCK_PARTITION_H__
#define __XSTAR_DRIVER_BLOCK_PARTITION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <driver/block/block.h>

int partition_detect(struct block_t * pblk);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_DRIVER_BLOCK_PARTITION_H__ */
