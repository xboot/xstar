# 时钟源 (clocksource)

时钟源。

## 设备类型

`DEVICE_TYPE_CLOCKSOURCE`

## 结构体

```c
struct clocksource_t {
    struct clocksource_keeper_t keeper;
    char * name;
    uint64_t mask;
    uint32_t mult, shift;
    uint64_t (*read)(struct clocksource_t * cs);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_clocksource(name)` | 按名称查找时钟源 |
| `register_clocksource(cs, drv)` | 注册时钟源 |
| `unregister_clocksource(cs)` | 注销时钟源 |
| `ktime_get()` | 获取系统内核时间 |

## 说明

自由运行单调计数器，使用高频硬件计数器和缩放数学提供连续时间基准。
