# 时钟事件定时器 (clockevent)

时钟事件定时器。

## 设备类型

`DEVICE_TYPE_CLOCKEVENT`

## 结构体

```c
struct clockevent_t {
    char * name;
    uint32_t mult, shift;
    uint64_t min_delta_ns, max_delta_ns;
    void * data;
    void (*handler)(struct clockevent_t * ce, void * data);
    int (*next)(struct clockevent_t * ce, uint64_t cycles);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_clockevent(name)` | 按名称查找时钟事件 |
| `register_clockevent(ce, drv)` | 注册时钟事件 |
| `unregister_clockevent(ce)` | 注销时钟事件 |
| `clockevent_set_event_handler(ce, handler, data)` | 设置处理函数 |
| `clockevent_set_event_next(ce, cycles)` | 设置下次触发 |

## 说明

可编程定时器事件设备。使用缩放数学在计数器滴答和纳秒之间转换。
