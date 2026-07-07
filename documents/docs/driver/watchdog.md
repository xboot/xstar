# 看门狗 (watchdog)

看门狗定时器。

## 设备类型

`DEVICE_TYPE_WATCHDOG`

## 结构体

```c
struct watchdog_t {
    char * name;
    int (*set)(struct watchdog_t * watchdog, int timeout);
    int (*get)(struct watchdog_t * watchdog);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_watchdog(name)` | 按名称查找看门狗 |
| `register_watchdog(wdt, drv)` | 注册看门狗 |
| `unregister_watchdog(wdt)` | 注销看门狗 |
| `watchdog_set_timeout(wdt, timeout)` | 设置超时（秒） |
| `watchdog_get_timeout(wdt)` | 获取剩余时间 |

## 说明

硬件看门狗定时器接口。超时值可配置，设置为 0 停止看门狗。用于安全关键和无人值守系统。
