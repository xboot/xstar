# LED (led)

LED。

## 设备类型

`DEVICE_TYPE_LED`

## 结构体

```c
struct led_t {
    char * name;
    int (*set)(struct led_t * led, int brightness);
    int (*get)(struct led_t * led);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_led(name)` | 按名称查找 LED |
| `register_led(led, drv)` | 注册 LED |
| `unregister_led(led)` | 注销 LED |
| `led_set_brightness(led, brightness)` | 设置亮度 |
| `led_get_brightness(led)` | 获取亮度 |

## 说明

简单 LED 亮度控制接口。亮度范围 0（灭）到 1000（全亮）。用于指示灯、状态 LED 和背光控制。
