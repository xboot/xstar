# 帧缓冲 (framebuffer)

帧缓冲显示。

## 设备类型

`DEVICE_TYPE_FRAMEBUFFER`

## 结构体

```c
struct framebuffer_t {
    char * name;
    int width, height, pwidth, pheight;
    int (*setbl)(struct framebuffer_t * fb, int brightness);
    int (*getbl)(struct framebuffer_t * fb);
    struct surface_t * (*create)(struct framebuffer_t * fb);
    void (*destroy)(struct framebuffer_t * fb, struct surface_t * s);
    void (*present)(struct framebuffer_t * fb, struct surface_t * s, struct region_list_t * l);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_framebuffer(name)` | 按名称查找帧缓冲 |
| `register_framebuffer(fb, drv)` | 注册帧缓冲 |
| `unregister_framebuffer(fb)` | 注销帧缓冲 |
| `framebuffer_get/set_backlight(fb, bl)` | 获取/设置背光 |

## 说明

视频帧缓冲显示接口。管理像素缓冲，支持 surface 创建和基于脏矩形的呈现。
