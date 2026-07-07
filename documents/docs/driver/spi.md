# SPI总线 (spi)

SPI 总线控制器。

## 设备类型

`DEVICE_TYPE_SPI`

## 结构体

```c
struct spi_msg_t {
    uint8_t * txbuf, * rxbuf;
    int len, mode, bits, speed;
};

struct spi_t {
    char * name;
    int type;
    int (*transfer)(struct spi_t * spi, struct spi_msg_t * msg);
    int (*select)(struct spi_t * spi, int cs);
    int (*deselect)(struct spi_t * spi, int cs);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_spi(name)` | 按名称查找 SPI 总线 |
| `register_spi(spi, drv)` | 注册 SPI 总线 |
| `unregister_spi(spi)` | 注销 SPI 总线 |
| `spi_transfer(spi, msg)` | 执行 SPI 传输 |
| `spi_device_write_then_read(dev, ...)` | SPI 设备先写后读 |

## 说明

SPI 总线控制器接口。支持单线/双线/四线/八线总线宽度，可配置模式、位数和速度。`spi_device_t` 句柄绑定总线和片选信号。
