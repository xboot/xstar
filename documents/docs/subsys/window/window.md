# 窗口 (window)

窗口子系统的核心模块，管理帧缓冲（framebuffer）和输入设备，提供帧循环（frame loop）、脏区域管理、事件队列和画面呈现功能，支持屏幕旋转和反射。

## 数据结构

```c
struct window_t {
    struct list_head_t list;        /* 全局窗口链表节点 */
    struct matrix2d_t lmatrix;      /* 本地变换矩阵 */
    struct matrix2d_t gmatrix;      /* 全局变换矩阵 */
    struct framebuffer_t * fb;      /* 帧缓冲设备 */
    struct surface_t * fbsurface[2];/* 帧缓冲坐标的呈现 Surface，由 fb->create 创建，单缓冲只用 [0] */
    struct surface_t * surface[2];  /* 软件绘制 Surface，单缓冲只用 [0]；为 NULL 时 acquire 直接返回 fbsurface */
    struct dirtylist_t * pending;   /* 当前帧累积的脏区域（窗口局部坐标） */
    struct dirtylist_t * inflight;  /* 上次 release 提交的脏区域 */
    struct dirtylist_t * fbspace[2];/* 交给驱动的脏区域（帧缓冲坐标） */
    struct mutex_t lock;            /* 帧状态转移锁 */
    struct fifo_t * event;          /* 事件 FIFO 队列 */
    struct hmap_t * map;            /* 输入设备映射表 */
    int bufdbl;                     /* 非零表示启用双缓冲 */
    int bufidx;                     /* 当前绘制缓冲索引 */
    int copyright;                  /* 版权标志 */
    int gmflag;                     /* 全局矩阵类型标志 */
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

旋转方向以逆时针为正。方向值为奇数（旋转 90/270 度或对角线翻转）时，窗口的宽高与帧缓冲互换，绘制 Surface 为 `surface_alloc()` 分配的转置软件面；180 度及水平/垂直翻转时为全尺寸软件面。除 `ROTATE_0` 外的所有方向都在 `window_alloc()` 时即创建软件绘制面。`ROTATE_0` 且从未调用过 `window_set_matrix()` 时，`surface[]` 保持为 NULL，acquire 直接返回 `fbsurface[]`，绘制即写呈现缓冲，支持页面翻转的驱动可零拷贝呈现，也省去了软件面的内存。首次调用 `window_set_matrix()` 时才分配软件绘制面并从 `fbsurface` 全量拷贝当前内容，此后无论矩阵如何变化都固定走软件面（`surface[0]` 非NULL 即软件面），该切换是单向的，避免矩阵在恒等与非恒等之间往复时反复整屏拷贝。

缓冲数量由 Kconfig 的 `Window buffer mode` 选项决定，默认选择 `Single buffer`，此时 `CONFIG_XSTAR_WINDOW_BUFFER_MODE` 为 0。选择 `Double buffer` 后该值为 1，窗口使用乒乓双缓冲加回拷，适合能异步完成呈现的驱动（如异步 DMA 或 DRM 页翻转）。同步驱动无法获得并行收益，且每帧增加脏区回拷（直绘 `fbsurface` 时一次，走软件面时另有帧缓冲面同步共两次），因此应保持默认的单缓冲配置。

每帧最多保留的脏矩形数量由 `CONFIG_XSTAR_WINDOW_DIRTY_RECTANGLE_LIMIT` 配置，取值范围为 1～32，默认值为 4。较小的值减少逐矩形处理和总线事务开销，较大的值则减少包围盒合并造成的过绘。

## 工作原理

### 帧循环

1. `window_frame_acquire()` 取得本帧的绘制 Surface：双缓冲模式直接返回上次 release 已切换的后台缓冲，单缓冲模式等待上一次提交不再被硬件引用
2. 在返回的 Surface 上执行图形绘制
3. `window_frame_damage()` 标记需要更新的区域（窗口局部坐标），必须在 acquire 与 release 之间调用
4. `window_frame_release()` 提交脏区域：先按 `CONFIG_XSTAR_WINDOW_DIRTY_RECTANGLE_LIMIT` 将脏矩形压缩至配置的上限，走软件面时再按全局矩阵把脏区变换到当前 `fbsurface`（可与上一帧 DMA 并行）；随后回收上一笔在途提交，将本帧脏区回拷到另一张绘制面，软件面路径还会把变换结果同步到另一张 `fbsurface`（使翻转型驱动的两个交替缓冲内容一致），随后转移脏区域列表的所有权并呈现，最后切换绘制缓冲。同步完成的提交会立即回收 `inflight`

未报告脏区域的 release 不产生任何驱动流量，天然支持跳帧。缓冲数量由编译期开关 `CONFIG_XSTAR_WINDOW_BUFFER_MODE` 决定（见“屏幕方向”一节）。

### 双缓冲流水线

双缓冲模式允许 CPU/GPU 绘制下一帧时，由 DMA 或显示控制器异步处理上一帧：

1. 在当前绘制面（如 `surface[0]`，直绘时为 `fbsurface[0]`）绘制第 N 帧
2. release 将脏区回拷到另一张绘制面，异步提交本帧呈现面，并将 `bufidx` 切换到 1
3. 下一次 acquire 立即返回切换后的绘制面，应用可以在硬件读取已提交内容时绘制第 N+1 帧
4. 走软件面时，第 N+1 帧 release 先按全局矩阵变换到本组 `fbsurface/fbspace`，并把结果回拷到另一组，此时仍可与第 N 帧 DMA 并行
5. 第 N+1 帧 release 在回拷、复用上一帧资源或发起下一笔提交前等待第 N 帧完成

要获得这种并行能力，release 必须传入非 NULL 回调以允许 framebuffer 驱动异步提交；`cb == NULL` 按接口约定强制同步完成。回拷发生在提交之前，因此备用 Surface 已包含上一帧完整内容，下一帧只需增量绘制。

### 脏区域管理

- `pending` 由应用在帧内累积，永远使用窗口局部坐标
- release 按 `CONFIG_XSTAR_WINDOW_DIRTY_RECTANGLE_LIMIT` 将 `pending` 压缩至配置的矩形数量上限，默认最多 4 个
- `inflight` 在 release 时与 `pending` 原子交换，归提交路径所有；同步提交立即清空，异步提交在下一次 retire、wait 或单缓冲 acquire 时等待完成后回收
- `fbspace[2]` 与 `fbsurface[2]` 按索引配对，保存变换到帧缓冲坐标的脏区域，加入前裁剪到 `fbsurface` 边界，驱动可假定不会越界；当驱动读取上一组时，CPU 可安全重建当前组

### 事件处理

- 输入设备驱动通过 `push_event_*()` 函数推送事件到全局事件队列
- `push_event()` 根据事件的 `device` 字段查找对应的窗口，将事件注入窗口的 FIFO 队列
- `window_pump_event()` 从窗口事件队列中取出事件供应用程序处理

### 屏幕变换

窗口通过本地变换矩阵（`lmatrix`）和全局变换矩阵（`gmatrix`）实现屏幕旋转和反射：

- `window_alloc()` 时根据 `orientation` 参数初始化变换矩阵
- `window_set_matrix()` 可在帧边界设置附加变换矩阵（全局矩阵 = 附加矩阵 × 本地矩阵）。首次设置建议紧接 `window_alloc()`、在第一次 `window_frame_acquire()` 前完成；不得在 `window_frame_acquire()` 与对应的 `window_frame_release()` 之间调用。若已有异步提交，应先调用 `window_frame_wait()`
- 走软件面时全局矩阵在 release 中自动应用，将脏区域从窗口坐标映射到帧缓冲坐标；直绘路径全局矩阵恒为恒等，无需变换

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

### 帧循环

| 函数 | 说明 |
|------|------|
| `window_frame_acquire(w)` | 取得本帧的绘制 Surface；`surface[0]` 为 NULL（初始 `ROTATE_0`）时返回 `fbsurface`，否则返回软件绘制面，单缓冲模式等待上一笔提交完成，双缓冲模式直接返回当前备用绘制面 |
| `window_frame_damage(w, r)` | 报告一个脏区域（窗口局部坐标），`r` 为 NULL 时表示整个窗口；须在 acquire 与 release 之间调用 |
| `window_frame_damage_get(w)` | 只读访问当前帧累积的脏区域列表，仅在本帧 release 前有效 |
| `window_frame_clear(w)` | 将当前绘制 Surface 的脏区域填充为默认棋盘格图案 |
| `window_frame_release(w, cb, data)` | 提交脏区域。`cb == NULL` 时同步阻塞直至完成；`cb != NULL` 时返回 1 表示传输在途（完成时调用 `cb(data)` 恰好一次，允许中断上下文），返回 0 表示已完成（不回调）。无脏区域时返回 0 且不做任何传输 |
| `window_frame_wait(w)` | 阻塞等待在途的异步呈现完成，并回收对应的 `inflight` 状态 |

### 属性

| 函数 | 说明 |
|------|------|
| `window_get_width(w)` | 获取窗口宽度（内联函数） |
| `window_get_height(w)` | 获取窗口高度（内联函数） |
| `window_get_pwidth(w)` | 获取物理宽度（毫米，内联函数） |
| `window_get_pheight(w)` | 获取物理高度（毫米，内联函数） |
| `window_get_dpi(w)` | 获取屏幕 DPI（内联函数） |
| `window_dp_to_px(w, dp)` | dp 转像素（内联函数） |
| `window_set_backlight(w, brightness)` | 设置背光亮度（内联函数） |
| `window_get_backlight(w)` | 获取背光亮度（内联函数） |
| `window_set_matrix(w, m)` | 在帧边界设置附加变换矩阵；首次调用会分配软件绘制面（含从 `fbsurface` 全量回拷当前内容），此后永久离开直绘零拷贝路径 |

### 事件

| 函数 | 说明 |
|------|------|
| `window_pump_event(w, e)` | 从事件队列取出一个事件，返回 1 表示有事件，0 表示队列为空 |
| `push_event(e)` | 将事件注入对应窗口的事件队列 |

## 用法示例

### 基本帧循环

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

    struct surface_t * s = window_frame_acquire(w);
    /* 在 s 上绘制内容 ... */

    window_frame_damage(w, NULL);
    window_frame_release(w, NULL, NULL);
}

window_free(w);
```

### 局部刷新

```c
struct surface_t * s = window_frame_acquire(w);
/* 只重绘变化的控件 */
window_frame_damage(w, &(struct region_t){ 32, 64, 128, 48 });
window_frame_release(w, NULL, NULL);
```

### 异步提交

```c
static void present_done(void * data)
{
    /* 本帧已呈现，可在此触发下一帧渲染 */
}

if(!window_frame_release(w, present_done, NULL))
	present_done(NULL); /* 返回 0 表示已同步完成，驱动不会回调 */

/* 双缓冲模式可立即 acquire 另一张 Surface，与异步呈现并行绘制 */
struct surface_t * next = window_frame_acquire(w);

/* 需要同步等待在途传输时（如销毁窗口前，window_free 内部会自动等待） */
window_frame_wait(w);
```

### 与渲染框架集成

上层渲染框架的“绘制缓冲已交付”与 framebuffer 的“DMA 已完成”是两个不同事件。以 LVGL direct 模式为例，适配层只注册一个逻辑 draw buffer，但在每次渲染开始时把它重新绑定到 `window_frame_acquire()` 返回的 Surface。最后一个 flush 使用非 NULL 空回调异步提交，然后立即通知 LVGL flush ready；真实 DMA 生命周期继续由 window 管理。这样 LVGL 可以开始绘制下一帧，而单缓冲模式仍会在下一次 acquire 中正确等待。

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
- 绘制 Surface 的像素格式为 32 位预乘 ARGB
- 脏区域机制避免全屏刷新，提高渲染效率；release 内部自动完成脏区域的合并优化
- 双缓冲模式下翻转后的回拷保证 acquire 返回的 Surface 总是上一帧的完整内容，应用只需绘制增量
- 内置水印被裁剪到本帧脏区域，确保实际修改、回拷和提交的像素范围一致
- 软件面路径按全局矩阵（旋转/反射/平移等）变换合成到 `fbsurface`，合成优先使用 G2D 硬件加速（若可用）；初始 `ROTATE_0` 未设置过矩阵时 acquire 直接返回 `fbsurface`，配合支持翻转的驱动实现零拷贝
- `framebuffer_create_surface()` 创建的始终是屏幕对应的 Surface，驱动可假定其返回的 Surface 与显示缓冲对应、能够直接呈现
- 异步呈现完成表示源 Surface 已不再被硬件引用，可以安全复用；具体驱动可将完成事件对齐 DMA 完成或垂直同步
- `window_pump_event()` 为非阻塞接口，无事件时立即返回 0
