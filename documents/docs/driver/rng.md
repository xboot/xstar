# 随机数生成器 (rng)

硬件随机数生成器。

## 设备类型

`DEVICE_TYPE_RNG`

## 结构体

```c
struct rng_t {
    char * name;
    int (*read)(struct rng_t * rng, uint32_t * buf, int len, int timeout);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_rng(name)` | 按名称查找 RNG |
| `register_rng(rng, drv)` | 注册 RNG |
| `unregister_rng(rng)` | 注销 RNG |
| `rng_read(rng, buf, len, timeout)` | 读取随机数 |

## 说明

硬件随机数生成器接口。从硬件熵源提供真随机数，用于加密密钥生成和 PRNG 种子。
