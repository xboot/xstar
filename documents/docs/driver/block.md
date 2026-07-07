# 块存储设备 (block)

块存储设备。

## 设备类型

`DEVICE_TYPE_BLOCK`

## 结构体

```c
struct block_t {
    char * name;
    uint64_t (*capacity)(struct block_t * block);
    int64_t (*read)(struct block_t * block, uint8_t * buf, uint64_t offset, uint64_t count);
    int64_t (*write)(struct block_t * block, uint8_t * buf, uint64_t offset, uint64_t count);
    void (*sync)(struct block_t * block);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_block(name)` | 按名称查找块设备 |
| `register_block(block, drv)` | 注册块设备 |
| `unregister_block(block)` | 注销块设备 |
| `register_sub_block(block, name, start, size)` | 注册子块设备（分区） |
| `block_capacity(block)` | 获取设备容量 |
| `block_read(block, buf, offset, count)` | 读取数据块 |
| `block_write(block, buf, offset, count)` | 写入数据块 |
| `block_sync(block)` | 同步数据 |

## 说明

块存储设备接口（Flash、磁盘、RAM 磁盘）。支持 64 位寻址和子块设备（分区）创建。
