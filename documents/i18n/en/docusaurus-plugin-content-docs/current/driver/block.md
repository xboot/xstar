# Block Storage Device (block)

Block storage device.

## Device Type

`DEVICE_TYPE_BLOCK`

## Structure

```c
struct block_t {
    char * name;
    uint64_t (*address)(struct block_t * block);
    uint64_t (*capacity)(struct block_t * block);
    uint64_t (*read)(struct block_t * block, uint8_t * buf, uint64_t offset, uint64_t count);
    uint64_t (*write)(struct block_t * block, uint8_t * buf, uint64_t offset, uint64_t count);
    void (*sync)(struct block_t * block);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_block(name)` | Find a block device by name |
| `register_block(block, drv)` | Register a block device |
| `unregister_block(block)` | Unregister a block device |
| `register_sub_block(pblk, offset, length, name)` | Register a sub-block device (partition) |
| `block_address(block)` | Get the direct memory base address of the device (optional) |
| `block_capacity(block)` | Get device capacity |
| `block_read(block, buf, offset, count)` | Read data blocks |
| `block_write(block, buf, offset, count)` | Write data blocks |
| `block_sync(block)` | Sync data |

## Description

Block storage device interface (Flash, disk, RAM disk). Supports 64-bit addressing and sub-block device (partition) creation.

The `address` callback is optional. It is only implemented when the device is backed by contiguous addressable memory (e.g. romdisk, ramdisk), returning the base address of the block in memory; returns 0 when unsupported. Sub-block devices automatically translate it to the parent block's base address plus offset.
