# DMA控制器 (dma)

DMA 控制器。

## 设备类型

`DEVICE_TYPE_DMACHIP`

## 结构体

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

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_dmachip(name)` | 按名称查找 DMA 控制器 |
| `register_dmachip(dmachip, drv)` | 注册 DMA 控制器 |
| `unregister_dmachip(dmachip)` | 注销 DMA 控制器 |
| `dma_start(channel, src, dst, count, flags)` | 启动 DMA 传输 |
| `dma_stop(channel)` | 停止 DMA 传输 |
| `dma_wait(channel)` | 等待传输完成 |

## 说明

DMA 控制器接口。支持多种传输方向和可配置数据宽度、突发大小。
