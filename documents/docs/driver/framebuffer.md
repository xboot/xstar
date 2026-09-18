# 帧缓冲 (framebuffer)

帧缓冲显示。

## 设备类型

`DEVICE_TYPE_FRAMEBUFFER`

## 结构体

```c
struct framebuffer_t {
    char * name;
    int width, height, pwidth, pheight;
    void (*setbl)(struct framebuffer_t * fb, int brightness);
    int (*getbl)(struct framebuffer_t * fb);
    struct surface_t * (*create)(struct framebuffer_t * fb, int width, int height);
    void (*destroy)(struct framebuffer_t * fb, struct surface_t * s);
    int (*present)(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l, void (*cb)(void *), void * data);
    void (*wait)(struct framebuffer_t * fb);
    void * priv;
};
```

## 呈现契约

`present` 统一支持同步与异步两种模式，完成语义为"源 surface 不再被硬件引用"：

- `cb == NULL`：同步模式，阻塞直到完成后返回 0
- `cb != NULL`：异步模式（建议性的）。返回 1 表示传输在途，完成时 `cb(data)` 恰好被调用一次（允许中断上下文）；返回 0 表示调用期间已完成，`cb` 不会被调用

`wait` 为必选实现，阻塞等待在途的 `present` 完成；不支持异步的驱动将其实现为空函数即可。页翻转类驱动（DRM、双缓冲 LCDC）的完成事件自然对齐垂直同步。

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_framebuffer(name)` | 按名称查找帧缓冲 |
| `register_framebuffer(fb, drv)` | 注册帧缓冲 |
| `unregister_framebuffer(fb)` | 注销帧缓冲 |
| `framebuffer_get/set_backlight(fb, bl)` | 获取/设置背光 |
| `framebuffer_present_submit(fb, s, l, cb, data)` | 提交呈现，`cb` 为 NULL 时同步阻塞直至完成 |
| `framebuffer_present_wait(fb)` | 等待在途的呈现完成 |

## 说明

视频帧缓冲显示接口。管理像素缓冲，支持 surface 创建和基于脏矩形的呈现，支持 DMA 异步刷新。
