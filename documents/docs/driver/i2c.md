# I2C总线 (i2c)

I2C 总线控制器。

## 设备类型

`DEVICE_TYPE_I2C` (23)

## 结构体

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

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_i2c(name)` | 按名称查找 I2C 总线 |
| `register_i2c(i2c, drv)` | 注册 I2C 总线 |
| `unregister_i2c(i2c)` | 注销 I2C 总线 |
| `i2c_transfer(i2c, msgs, num)` | 执行 I2C 消息传输 |
| `i2c_master_send(dev, buf, count)` | I2C 主机发送 |
| `i2c_master_recv(dev, buf, count)` | I2C 主机接收 |

## 说明

I2C 总线控制器接口。支持多消息传输，可配置 10 位寻址、重复起始等标志。`i2c_device_t` 句柄将总线名和从机地址绑定为方便的主机发送/接收操作。
