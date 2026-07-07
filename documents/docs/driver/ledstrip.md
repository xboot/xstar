# LED灯带 (ledstrip)

可寻址 LED 灯带。

## 设备类型

`DEVICE_TYPE_LEDSTRIP`

## 结构体

```c
struct ledstrip_t {
    char * name;
    int (*set_count)(struct ledstrip_t * strip, int count);
    int (*get_count)(struct ledstrip_t * strip);
    int (*set_color)(struct ledstrip_t * strip, int idx, struct color_t * c);
    int (*get_color)(struct ledstrip_t * strip, int idx, struct color_t * c);
    int (*refresh)(struct ledstrip_t * strip);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_ledstrip(name)` | 按名称查找 LED 灯带 |
| `register_ledstrip(strip, drv)` | 注册 LED 灯带 |
| `unregister_ledstrip(strip)` | 注销 LED 灯带 |
| `ledstrip_set_color(strip, idx, color)` | 设置像素颜色 |
| `ledstrip_refresh(strip)` | 刷新灯带显示 |

## 说明

可寻址 LED 灯带接口（如 WS2812/NeoPixel）。每个像素的颜色通过 `struct color_t` 设置，通过 `refresh()` 提交更改。
