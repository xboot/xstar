# 非易失性存储器 (nvmem)

非易失性存储器。

## 设备类型

`DEVICE_TYPE_NVMEM`

## 结构体

```c
struct nvmem_t {
    char * name;
    uint64_t (*capacity)(struct nvmem_t * nvmem);
    int64_t (*read)(struct nvmem_t * nvmem, uint8_t * buf, uint64_t offset, uint64_t count);
    int64_t (*write)(struct nvmem_t * nvmem, uint8_t * buf, uint64_t offset, uint64_t count);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_nvmem(name)` | 按名称查找 NVMEM 设备 |
| `register_nvmem(nvmem, drv)` | 注册 NVMEM 设备 |
| `unregister_nvmem(nvmem)` | 注销 NVMEM 设备 |
| `nvmem_capacity(nvmem)` | 获取容量 |
| `nvmem_read(nvmem, buf, offset, count)` | 读取数据 |
| `nvmem_write(nvmem, buf, offset, count)` | 写入数据 |

## 说明

非易失性存储器（EEPROM、FRAM）接口。提供字节可寻址的读写操作，用于存储配置数据、校准值和设备身份信息。
