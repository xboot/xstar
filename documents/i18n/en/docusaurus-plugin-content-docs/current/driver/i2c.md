# I2C Bus (i2c)

I2C bus controller.

## Device Type

`DEVICE_TYPE_I2C` (23)

## Struct

```c
struct i2c_msg_t {
    uint16_t addr;
    uint16_t flags;
    int16_t len;
    uint8_t * buf;
};

struct i2c_t {
    char * name;
    int (*xfer)(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num);
    void * priv;
};
```

## Key API

| Function | Description |
|------|------|
| `search_i2c(name)` | Find I2C bus by name |
| `register_i2c(i2c, drv)` | Register I2C bus |
| `unregister_i2c(i2c)` | Unregister I2C bus |
| `i2c_transfer(i2c, msgs, num)` | Execute I2C message transfer |
| `i2c_master_send(dev, buf, count)` | I2C master send |
| `i2c_master_recv(dev, buf, count)` | I2C master receive |

## Description

I2C bus controller interface. Supports multi-message transfer with configurable flags such as 10-bit addressing and repeated start. The `i2c_device_t` handle binds bus name and slave address for convenient master send/receive operations.
