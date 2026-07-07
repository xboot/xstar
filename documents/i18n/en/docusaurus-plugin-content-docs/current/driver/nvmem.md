# Non-Volatile Memory (nvmem)

Non-volatile memory.

## Device Type

`DEVICE_TYPE_NVMEM`

## Structure

```c
struct nvmem_t {
    char * name;
    uint64_t (*capacity)(struct nvmem_t * nvmem);
    int64_t (*read)(struct nvmem_t * nvmem, uint8_t * buf, uint64_t offset, uint64_t count);
    int64_t (*write)(struct nvmem_t * nvmem, uint8_t * buf, uint64_t offset, uint64_t count);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_nvmem(name)` | Find NVMEM device by name |
| `register_nvmem(nvmem, drv)` | Register NVMEM device |
| `unregister_nvmem(nvmem)` | Unregister NVMEM device |
| `nvmem_capacity(nvmem)` | Get capacity |
| `nvmem_read(nvmem, buf, offset, count)` | Read data |
| `nvmem_write(nvmem, buf, offset, count)` | Write data |

## Description

Non-volatile memory (EEPROM, FRAM) interface. Provides byte-addressable read and write operations for storing configuration data, calibration values, and device identity information.
