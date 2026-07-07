# DMA Controller (dma)

DMA controller.

## Device Type

`DEVICE_TYPE_DMACHIP`

## Struct

```c
struct dmachip_t {
    char * name;
    int base, ndma, channel;
    void (*start)(struct dmachip_t * dmachip, int c, void * s, void * d, int64_t n, uint64_t f);
    void (*stop)(struct dmachip_t * dmachip, int c);
    int (*busying)(struct dmachip_t * dmachip, int c);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_dmachip(name)` | Find DMA controller by name |
| `register_dmachip(dmachip, drv)` | Register DMA controller |
| `unregister_dmachip(dmachip)` | Unregister DMA controller |
| `dma_start(channel, src, dst, count, flags)` | Start DMA transfer |
| `dma_stop(channel)` | Stop DMA transfer |
| `dma_wait(channel)` | Wait for transfer completion |

## Description

DMA controller interface. Supports multiple transfer directions and configurable data width, burst size.
