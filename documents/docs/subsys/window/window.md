# 窗口 (window)

窗口子系统的核心模块，管理帧缓冲（framebuffer）和输入设备，提供渲染 Surface、脏矩形管理、事件队列和画面呈现功能，支持屏幕旋转和反射。

## 数据结构

```c
struct window_t {
    struct list_head_t list;        /* 全局窗口链表节点 */
    struct matrix2d_t lmatrix;      /* 本地变换矩阵 */
    struct matrix2d_t gmatrix;      /* 全局变换矩阵 */
    struct framebuffer_t * fb;      /* 帧缓冲设备 */
    struct surface_t * fbsurface;   /* 帧缓冲 Surface */
    struct surface_t * surface;     /* 渲染 Surface */
    struct dirtylist_t * dirtylist; /* 脏矩形列表 */
    struct fifo_t * event;          /* 事件 FIFO 队列 */
    struct hmap_t * map;            /* 输入设备映射表 */
    int copyright;                  /* 版权标志 */
    int gmflag;                     /* 全局矩阵更新标志 */
    int dpi;                        /* 屏幕 DPI */
};
```

## 屏幕方向

```c
enum window_orientation_t {
    WINDOW_ORIENTATION_ROTATE_0    = 0,  /* 0 度（默认方向） */
    WINDOW_ORIENTATION_ROTATE_90   = 1,  /* 逆时针 90 度 */
    WINDOW_ORIENTATION_ROTATE_180  = 2,  /* 逆时针 180 度 */
    WINDOW_ORIENTATION_ROTATE_270  = 3,  /* 逆时针 270 度 */
    WINDOW_ORIENTATION_FLIP_H      = 4,  /* 水平翻转 */
    WINDOW_ORIENTATION_FLIP_MD     = 5,  /* 主对角线翻转 */
    WINDOW_ORIENTATION_FLIP_V      = 6,  /* 垂直翻转 */
    WINDOW_ORIENTATION_FLIP_AD     = 7,  /* 反对角线翻转 */
};
```

旋转方向以逆时针为正。当屏幕旋转 90 或 270 度时，窗口的宽高与帧缓冲的宽高互换。

## 工作原理

### 渲染流程

1. `window_alloc()` 创建窗口，绑定帧缓冲设备和输入设备，分配渲染 Surface
2. 通过 `window_get_surface()` 获取渲染 Surface，在其上执行图形绘制
3. `window_dirtylist_add()` 标记需要更新的区域
4. `window_present_commit()` 将脏区域内的内容从渲染 Surface 合成到帧缓冲，并调用 `framebuffer_present()` 刷新屏幕
5. `window_present_clear()` 清空脏矩形列表，准备下一帧

### 事件处理

- 输入设备驱动通过 `push_event_*()` 函数推送事件到全局事件队列
- `push_event()` 根据事件的 `device` 字段查找对应的窗口，将事件注入窗口的 FIFO 队列
- `window_pump_event()` 从窗口事件队列中取出事件供应用程序处理

### 屏幕变换

窗口通过本地变换矩阵（`lmatrix`）和全局变换矩阵（`gmatrix`）实现屏幕旋转和反射：

- `window_alloc()` 时根据 `orientation` 参数初始化变换矩阵
- `window_set_matrix()` 可运行时修改本地变换矩阵
- 全局矩阵在渲染时自动应用，将渲染 Surface 的坐标映射到帧缓冲坐标

### DPI 与单位转换

窗口维护 DPI（每英寸像素数）信息，支持 dp 到像素的转换：

```c
int px = window_dp_to_px(w, 16);  /* 16dp 转换为像素 */
```

转换公式：`px = max(dpi * dp / 160, 1)`

## API

### 生命周期

| 函数 | 说明 |
|------|------|
| `window_alloc(fb, input, orientation)` | 创建窗口，`fb` 为帧缓冲设备名，`input` 为输入设备名，`orientation` 为屏幕方向 |
| `window_free(w)` | 销毁窗口，释放所有资源 |
| `window_exit(w)` | 请求窗口退出 |

### 属性

| 函数 | 说明 |
|------|------|
| `window_get_surface(w)` | 获取渲染 Surface（内联函数） |
| `window_get_width(w)` | 获取窗口宽度（内联函数） |
| `window_get_height(w)` | 获取窗口高度（内联函数） |
| `window_get_pwidth(w)` | 获取物理宽度（毫米，内联函数） |
| `window_get_pheight(w)` | 获取物理高度（毫米，内联函数） |
| `window_get_dpi(w)` | 获取屏幕 DPI（内联函数） |
| `window_dp_to_px(w, dp)` | dp 转像素（内联函数） |
| `window_set_backlight(w, brightness)` | 设置背光亮度（内联函数） |
| `window_get_backlight(w)` | 获取背光亮度（内联函数） |
| `window_set_matrix(w, m)` | 设置本地变换矩阵 |

### 脏矩形与呈现

| 函数 | 说明 |
|------|------|
| `window_dirtylist_fullscreen(w)` | 标记全屏为脏区域 |
| `window_dirtylist_clear(w)` | 清空脏矩形列表 |
| `window_dirtylist_add(w, r)` | 添加区域到脏矩形列表 |
| `window_present_clear(w)` | 清空脏矩形并清除渲染 Surface |
| `window_present_commit(w)` | 提交脏区域到帧缓冲并呈现 |

### 事件

| 函数 | 说明 |
|------|------|
| `window_pump_event(w, e)` | 从事件队列取出一个事件，返回 1 表示有事件，0 表示队列为空 |
| `push_event(e)` | 将事件注入对应窗口的事件队列 |

## 用法示例

### 基本窗口循环

```c
#include <kernel/window/window.h>

struct window_t * w = window_alloc("fb-linux-sdl.0", "input-linux.0", WINDOW_ORIENTATION_ROTATE_0);

struct event_t e;
while(window_pump_event(w, &e) || 1)
{
    while(window_pump_event(w, &e))
    {
        if(e.type == EVENT_TYPE_KEY_DOWN && e.e.key_down.key == KB_KEY_BACK)
            window_exit(w);
    }

    struct surface_t * s = window_get_surface(w);
    /* 在 s 上绘制内容 ... */

    window_dirtylist_fullscreen(w);
    window_present_commit(w);
}

window_free(w);
```

### 屏幕旋转

```c
/* 逆时针旋转 90 度 */
struct window_t * w = window_alloc("fb.0", "input.0", WINDOW_ORIENTATION_ROTATE_90);
/* window_get_width/height 返回的是旋转后的尺寸 */
```

### 背光控制

```c
window_set_backlight(w, 500);  /* 设置背光为 50%（范围 0-1000） */
int bl = window_get_backlight(w);
```

## 说明

- 若指定的帧缓冲设备不存在，自动使用 `fb-dummy`（128x128 虚拟帧缓冲）
- 渲染 Surface 的像素格式为 32 位预乘 ARGB
- 脏矩形机制避免全屏刷新，提高渲染效率
- `window_present_commit()` 内部通过 G2D 硬件加速（若可用）执行 Surface 合成
- `window_pump_event()` 为非阻塞接口，无事件时立即返回 0
