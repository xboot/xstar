# LED触发器 (ledtrigger)

LED 触发器。

## 设备类型

`DEVICE_TYPE_LEDTRIGGER`

## 结构体

```c
struct ledtrigger_t {
    char * name;
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_ledtrigger(name)` | 按名称查找 LED 触发器 |
| `register_ledtrigger(trig, drv)` | 注册 LED 触发器 |
| `unregister_ledtrigger(trig)` | 注销 LED 触发器 |

## 说明

LED 触发器关联设备。将命名触发器（如 "heartbeat"、"mmc"）链接到 LED，使 LED 根据系统事件自动改变状态。
