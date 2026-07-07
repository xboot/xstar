# SPI Bus (spi)

SPI bus controller.

## Device Type

`DEVICE_TYPE_SPI`

## Structure

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

## Key API

| Function | Description |
|------|------|
| `search_spi(name)` | Find SPI bus by name |
| `register_spi(spi, drv)` | Register an SPI bus |
| `unregister_spi(spi)` | Unregister an SPI bus |
| `spi_transfer(spi, msg)` | Execute SPI transfer |
| `spi_device_write_then_read(dev, ...)` | SPI device write then read |

## Description

SPI bus controller interface. Supports single/dual/quad/octuple bus widths, configurable mode, bit width and speed. The `spi_device_t` handle binds a bus and chip select signal.
