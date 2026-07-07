# XSTAR 架构设计 更新一下呢？by jjj

本文档详细说明 XSTAR 的系统架构和核心组件设计。

## 目录

- [整体架构](#整体架构)
- [启动流程](#启动流程)
- [跨平台抽象层 (XOS)](#跨平台抽象层-xos)
- [设备驱动框架](#设备驱动框架)
- [内核子系统](#内核子系统)
- [协程系统](#协程系统)
- [图形系统](#图形系统)
- [KOBJ 虚拟文件系统](#kobj-虚拟文件系统)
- [发布订阅系统](#发布订阅系统)
- [Initcall 机制](#initcall-机制)
- [设备树 (JSON)](#设备树-json)
- [工具库 (LibX)](#工具库-libx)
- [构建系统](#构建系统)

## 整体架构

XSTAR 采用分层架构设计，从底层到上层依次为：

```
┌──────────────────────────────────────────────────────────────────┐
│                       应用层 (Applications)                       │
├──────────────────────────────────────────────────────────────────┤
│              内核子系统 (Kernel Subsystems)                        │
│  Audio | Command | Core | Font | Graphic | Shell | Time          │
│  Vision | Window | XFS                                           │
├──────────────────────────────────────────────────────────────────┤
│               设备驱动框架 (Driver Framework)                      │
│          Driver | Device | Class | KOBJ | DTREE (JSON)            │
├──────────────────────────────────────────────────────────────────┤
│                 平台抽象层 (XOS)                                   │
│  Memory | DMA | I/O | File | Thread | Mutex | Semaphore          │
│  Coroutine | PM | Stdio                                          │
├──────────────────────────────────────────────────────────────────┤
│                  工具库 (LibX)                                     │
│  Algorithm | Data Structure | Crypto | JSON | DTREE | Encoding   │
├──────────────────────────────────────────────────────────────────┤
│          硬件平台 (Hardware Platforms)                              │
│  ARM32/64 | RISC-V32/64 | x64 (Baremetal/Linux/FreeRTOS)         │
└──────────────────────────────────────────────────────────────────┘
```

### 分层说明

1. **硬件平台层**：提供基础的硬件支持，包括 CPU、内存、外设等
2. **LibX 工具库**：提供通用的算法、数据结构、加密、编码等工具函数
3. **XOS 抽象层**：屏蔽底层差异，提供统一的系统调用接口
4. **设备驱动框架**：管理设备驱动和设备实例，实现驱动的自动探测和注册
5. **内核子系统**：提供音频、图形、命令、Shell、时间、视觉、窗口、文件系统等核心功能
6. **应用层**：基于内核子系统构建的上层应用

## 启动流程

系统入口为 `xstar_init()`（`xstar/xstar.c`），平台特定的 `main()` 调用 `xstar_init(&env, json)` 完成整个系统初始化：

```
main() → xstar_init()
  ├── xos_environ_init(env)         安装平台抽象函数表
  ├── do_initcalls()                按级别执行所有 initcall (0→9)
  ├── do_init_romdisk()             注册 romdisk 块设备
  ├── do_init_dtree(dtree)         解析设备树 JSON，探测所有设备
  ├── do_init_memory()              注册内存信息 KOBJ
  ├── do_init_logger()              注册日志控制 KOBJ
  ├── do_init_version()             注册版本信息 KOBJ
  ├── do_init_copyright()           注册版权验证 KOBJ
  ├── do_init_random()              初始化随机数生成器
  ├── do_init_feature()             检测协程/线程支持特性
  ├── do_init_font()                加载 TrueType 字体
  ├── do_init_setting()             初始化持久化设置系统
  └── do_show_logo()                在帧缓冲上显示启动 Logo
```

关键步骤说明：

- **`xos_environ_init(env)`**：将平台提供的函数指针安装到全局 `__xos_environ` 结构体中，仅覆盖非 NULL 的条目
- **`do_initcalls()`**：按级别 0-9 顺序执行链接器段中的初始化函数，驱动注册、子系统初始化、命令注册都在此阶段完成
- **`do_init_dtree(dtree)`**：解析设备树 JSON，对每个设备节点匹配驱动并调用 `probe()`，创建设备实例

## 跨平台抽象层 (XOS)

XOS 是 XSTAR 的核心抽象层，通过函数指针表屏蔽不同平台和运行环境的差异。

### 接口抽象

```c
struct xos_environ_t {
    /* 内存管理 */
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
    void *(*realloc)(void *ptr, size_t size);

    /* DMA 操作 */
    int (*dma_alloc)(void *addr, size_t size, uint64_t pa);
    int (*dma_free)(void *addr, size_t size);

    /* IO 操作 */
    void (*write8)(io_addr_t addr, uint8_t value);
    void (*write16)(io_addr_t addr, uint16_t value);
    void (*write32)(io_addr_t addr, uint32_t value);
    uint8_t (*read8)(io_addr_t addr);
    uint16_t (*read16)(io_addr_t addr);
    uint32_t (*read32)(io_addr_t addr);

    /* 文件系统 */
    void *(*fopen)(const char *path, const char *mode);
    int (*fclose)(void *file);
    size_t (*fread)(void *ptr, size_t size, size_t nmemb, void *file);
    size_t (*fwrite)(const void *ptr, size_t size, size_t nmemb, void *file);

    /* 协程 */
    void (*coroutine_make)(void *stack, size_t size, void (*func)(struct co_transfer_t));
    struct co_transfer_t (*coroutine_jump)(void *fctx, void *priv);

    /* 线程 */
    void *(*thread_create)(const char *name, void (*func)(void *), void *data, int stksz);
    void (*thread_destroy)(void *thread);
    void (*thread_wait)(void *thread);
    void (*thread_sleep)(uint64_t ns);

    /* 互斥锁 */
    int (*mutex_init)(void *mutex);
    int (*mutex_lock)(void *mutex);
    int (*mutex_unlock)(void *mutex);
    int (*mutex_exit)(void *mutex);

    /* 信号量 */
    int (*semaphore_init)(void *sem, uint32_t count);
    int (*semaphore_wait)(void *sem, uint32_t timeout);
    int (*semaphore_post)(void *sem);
    int (*semaphore_exit)(void *sem);

    /* 电源管理 */
    void (*shutdown)(void);
    void (*reboot)(void);
    void (*standby)(void);
};
```

### API 分类

XOS 通过 `xos_environ_t` 提供平台相关的操作，同时还直接提供大量平台无关的可移植 API：

| 分类 | 关键函数 |
|------|---------|
| **内存管理** | `xos_mem_malloc`, `xos_mem_free`, `xos_mem_realloc`, `xos_mem_calloc`, `xos_mem_memalign`, `xos_mem_meminfo` |
| **DMA 操作** | `xos_dma_alloc_coherent`, `xos_dma_free_coherent`, `xos_dma_alloc_noncoherent`, `xos_dma_sync` |
| **硬件 I/O** | `xos_io_read8/16/32/64`, `xos_io_write8/16/32/64`, `xos_io_clrbits/setbits/clrsetbits` |
| **标准 I/O** | `xos_stdio_read`, `xos_stdio_write` |
| **文件系统** | `xos_file_open/close/read/write/seek`, `xos_file_mkdir/remove/walk`, `xos_file_isdir/isfile` |
| **协程** | `xos_coroutine_make`, `xos_coroutine_jump` |
| **线程** | `xos_thread_create/destroy/wait/sleep` |
| **互斥锁** | `xos_mutex_init/exit/lock/unlock/trylock` |
| **信号量** | `xos_semaphore_init/exit/wait/post` |
| **电源管理** | `xos_pm_shutdown/reboot/standby` |
| **字符串操作** | `xos_strcmp/strcpy/strcat/strlen/strstr/strdup/strtok` 等 |
| **格式化** | `xos_sprintf/snprintf/printf/sscanf` 等 |
| **数值转换** | `xos_strtol/strtoll/strtod/atoi/atol` 等 |
| **排序搜索** | `xos_qsort`, `xos_bsearch` |
| **随机数** | `xos_srand/rand/random_int/random_float` |

### 平台实现

每个平台提供对应的 `xos_environ_t` 实现：

- `xos-linux.c`：Linux/SDL 平台实现
- `xos-baremetal-arm64.c`：ARM64 裸机实现
- `xos-baremetal-arm32.c`：ARM32 裸机实现
- `xos-baremetal-riscv64.c`：RISC-V64 裸机实现
- `xos-baremetal-riscv32.c`：RISC-V32 裸机实现
- `xos-freertos.c`：FreeRTOS 平台实现
- `xos-windows.c`：Windows 平台实现

## 设备驱动框架

设备驱动框架采用驱动/设备分离设计，实现驱动的自动探测和注册。

### 核心数据结构

```c
struct driver_t {
    struct kobj_t * kobj;
    struct hlist_node_t node;
    char * name;
    struct device_t * (*probe)(struct driver_t * drv, struct dtnode_t * n);
    void (*remove)(struct device_t * dev);
    void (*suspend)(struct device_t * dev);
    void (*resume)(struct device_t * dev);
};

struct device_t {
    struct kobj_t * kobj;
    struct list_head_t list;
    struct list_head_t head;
    struct hlist_node_t node;
    char * name;
    enum device_type_t type;
    struct driver_t * driver;
    void * priv;
};
```

### 设备类型

系统定义了 51 种设备类型（`enum device_type_t`）：

| 类别 | 类型 |
|------|------|
| 时钟 | `CLK`, `CLOCKEVENT`, `CLOCKSOURCE` |
| 存储 | `BLOCK`, `NVMEM`, `SDHCI` |
| 显示 | `FRAMEBUFFER`, `G2D`, `CONSOLE` |
| 音频 | `AUDIOCAPTURE`, `AUDIOPLAYBACK`, `BUZZER` |
| 输入 | `INPUT`, `CAMERA` |
| 通信 | `I2C`, `SPI`, `UART`, `NET`, `ATNET`, `GNSS` |
| GPIO | `GPIOCHIP`, `IRQCHIP`, `RESETCHIP` |
| 传感器 | `ADC`, `COMPASS`, `GMETER`, `GYROSCOPE`, `HYGROMETER`, `LIGHT`, `OXIMETER`, `PRESSURE`, `PROXIMITY`, `THERMOMETER` |
| 输出 | `LED`, `LEDSTRIP`, `LEDTRIGGER`, `DAC`, `PWM`, `SERVO`, `MOTOR`, `STEPPER`, `VIBRATOR` |
| 电源 | `BATTERY`, `REGULATOR`, `RNG`, `RTC` |
| 其他 | `ATOMIC`, `DMA`, `LIMITER`, `PRINTER`, `SPINLOCK`, `WATCHDOG` |

### 驱动注册

驱动通过 `driver_initcall` 宏自动注册：

```c
static struct driver_t my_driver = {
    .name    = "my-driver",
    .probe   = my_driver_probe,
    .remove  = my_driver_remove,
    .suspend = my_driver_suspend,
    .resume  = my_driver_resume,
};

static void my_driver_init(void)
{
    register_driver(&my_driver);
}

static void my_driver_exit(void)
{
    unregister_driver(&my_driver);
}

driver_initcall(my_driver_init);
driver_exitcall(my_driver_exit);
```

### 驱动查找

驱动存储在哈希表中（521 个桶，使用 `shash()` 哈希函数），查找效率接近 O(1)：

- `register_driver(drv)`：创建 KOBJ 目录 `/class/driver/<name>/`，添加 `probe` 写入入口，插入哈希表
- `unregister_driver(drv)`：从哈希表和 KOBJ 树中移除
- `search_driver(name)`：按名称在哈希表中查找驱动

### 设备注册

设备通过三重索引进行管理：

- 全局链表 `__device_list`
- 按类型链表 `__device_head[type]`
- 按名称哈希表

`register_device(dev)` 添加设备到三重索引，并通过 `psub_publish("device.add", dev)` 发布设备添加事件。

### 设备探测

系统启动时，`probe_device()` 解析设备树 JSON，对每个节点自动匹配驱动并探测：

```
解析 JSON → 提取键名 "driver-name:id@addr" → search_driver(name)
→ 调用 drv->probe(drv, n) → 创建设备实例 → 注册到系统
```

## 内核子系统

### 音频系统 (Audio)

完整的音频处理链路：

```
Source (音频源) → Mixer (混音器) → Effect (音效处理) → Sink (输出设备)
```

- **Source**：音频输入抽象，支持文件（WAV、QOA）、捕获设备、内存缓冲、AFSK 调制解调、音调生成器、噪声生成器、混音器
  - 关键 API：`audio_source_alloc_from_xfs()`, `audio_source_read()`, `audio_source_seek()`
- **Mixer**：多路音频混音，混音器本身也可作为 Source 使用
  - 关键 API：`audio_mixer_alloc()`, `audio_mixer_add/remove()`, `audio_mixer_read()`
- **Effect**：可插拔的音效过滤器链，支持 crystalizer、duplicate、IIR、mono、panning、resample、reshape、tremolo、volume 等效果
  - 关键 API：`audio_filter_alloc(json, len)`, `audio_effect_process()`
- **Sink**：音频输出抽象，支持播放设备、幅度计、频谱分析、VAD 语音活动检测、AFSK 解码
  - 关键 API：`audio_sink_alloc_from_playback()`, `audio_sink_write()`, `audio_sink_ioctl()`

### 命令系统 (Command)

统一的命令接口，支持 32+ 内置命令：

```c
struct command_t {
    char * name;
    char * desc;
    void (*usage)(void);
    void (*exec)(int argc, char **argv);
};
```

内置命令包括：aplay、autoshell、cat、cd、clear、clk、date、dcp、delay、echo、event、go、help、iplay、ls、md、mkdir、mw、net、ntpdate、pwd、reboot、rm、setting、shutdown、standby、sync、test、tscal、uniqueid、version、write。

### 核心工具 (Core)

| 模块 | 说明 | 关键 API |
|------|------|---------|
| **Coroutine** | 协作式多任务 | `coroutine_start()`, `coroutine_yield()`, `coroutine_msleep()` |
| **Logger** | 循环缓冲区日志 | `LOG(fmt, ...)`, `logger_enable/disable()` |
| **Profiler** | 性能分析 | `profiler_begin/end()`, `profiler_search()`, `profiler_foreach()` |
| **Setting** | 持久化键值存储 | `setting_set/get/clear/sync/foreach()` |
| **ThChannel** | 线程安全环形缓冲通道 | `thchannel_alloc()`, `thchannel_send/recv(buf, len, timeout)` |
| **ThWorker** | 线程工作队列 | `thworker_alloc()`, `thworker_submit(func, data)`, `thworker_wait()` |
| **CoChannel** | 协程安全通道（无锁） | `cochannel_alloc()`, `cochannel_send/recv(sched, buf, len)` |
| **PSub** | 发布订阅系统 | `psub_publish(topic, data)`, `psub_subscribe(topic, cb, oneshot)` |

### 字体系统 (Font)

支持 4 种字型（regular、italic、bold、bolditalic）的字体管理：

- **Font**：字体族管理，从 XFS 或缓冲区安装字体
  - 关键 API：`font_install_from_xfs/buf()`, `font_text_bound/render()`, `font_icon_bound/render()`
- **TrueType**：TrueType/CFF 字体解析器
  - 关键 API：`truetype_init()`, `truetype_scale_for_pixel_height()`, `truetype_make_glyph_bitmap()`

### Shell 系统 (Shell)

交互式 Shell，支持：

- 命令自动补全
- 历史记录导航
- Ctrl+C 中断处理
- 工作目录管理

关键 API：`shell_system(cmdline)`, `shell_readline(prompt)`, `shell_password(prompt)`, `shell_getcwd/setcwd()`

### 时间系统 (Time)

高精度时间管理：

| 模块 | 说明 | 关键 API |
|------|------|---------|
| **Timer** | 基于红黑树的高精度定时器 | `timer_init()`, `timer_start()`, `timer_forward()`, `timer_cancel()` |
| **WallClock** | 墙钟时间 | `wallclock_gettimeofday/settimeofday()`, `wallclock_gettime/settime()` |
| **Delay** | 忙等待延时 | `ndelay(ns)`, `udelay(us)`, `mdelay(ms)` |
| **DelayCall** | 一次性延时调用 | `delaycall(ms, func, data)` |

### 视觉系统 (Vision)

图像处理算法库，支持两种像素格式：`VISION_TYPE_GRAY`（8 位灰度）和 `VISION_TYPE_RGB`（24 位 RGB888）。

```c
struct vision_t {
    enum vision_type_t type;
    int width;
    int height;
    int npixel;
    void * datas;
    size_t ndata;
};
```

关键 API：`vision_alloc()`, `vision_free()`, `vision_clone()`, `vision_convert()`, `vision_clear()`

支持的操作包括：bitwise（位运算）、colormap（色彩映射）、dilate（膨胀）、erode（腐蚀）、threshold（阈值化）、gamma（伽马校正）、gray（灰度化）、sepia（怀旧色调）、inrange（范围过滤）、invert（反转）、resize（缩放）、dither（抖动）、rectangle（矩形绘制）、text（文本渲染）。

视觉系统与图形系统可互相转换：`vision_apply_surface()` / `surface_apply_vision()`。

### 窗口系统 (Window)

窗口管理和事件处理：

```c
struct window_t {
    struct list_head_t list;
    struct matrix2d_t lmatrix;
    struct matrix2d_t gmatrix;
    struct framebuffer_t * fb;
    struct surface_t * fbsurface;
    struct surface_t * surface;
    struct dirtylist_t * dirtylist;
    struct fifo_t * event;
    struct hmap_t * map;
    int dpi;
};
```

- 窗口创建与管理：`window_alloc()`, `window_free()`, `window_exit()`
- 屏幕方向：支持 4 种旋转（0/90/180/270）和 4 种翻转（水平/垂直/主对角线/反对角线）
- 脏矩形管理：`window_dirtylist_fullscreen()`, `window_dirtylist_add()`, `window_dirtylist_clear()`
- 渲染提交：`window_present_clear()`, `window_present_commit()`
- 事件泵：`window_pump_event()`, `push_event()`
- 背光控制：`window_set_backlight()`, `window_get_backlight()`
- dp/px 转换：`window_dp_to_px()`

### 文件系统 (XFS)

虚拟文件系统，支持多挂载点：

```c
struct xfs_context_t {
    struct xfs_path_t mounts;
    struct mutex_t lock;
};
```

关键 API：

- 挂载管理：`xfs_mount()`, `xfs_umount()`
- 文件操作：`xfs_open_read/write/append()`, `xfs_read/write/seek/tell/length()`, `xfs_flush()`, `xfs_close()`
- 目录操作：`xfs_walk()`, `xfs_isdir/isfile()`, `xfs_mkdir()`, `xfs_remove()`
- 归档器：`xfs_archiver_t` 提供可插拔的归档格式支持

## 协程系统

XSTAR 提供汇编级协程实现，支持多种架构。

### 协程上下文

为每种架构提供完整的上下文保存和恢复：

- **ARM32**：保存 R0-R15、CPSR 等
- **ARM64**：保存 X0-X30、SP、PC、PSTATE 等
- **RISC-V32**：保存 x0-x31、SP、PC 等
- **RISC-V64**：保存 x0-x31、SP、PC 等
- **x64**：保存 RAX-R15、RSP、RIP 等

### 协程调度器

```c
struct scheduler_t {
    struct list_head_t runlist;
    struct coroutine_t * running;
    struct mutex_t * mutex;
};
```

### 协程 API

```c
void coroutine_start(struct scheduler_t *sched, void (*func)(void *), void *data, size_t ssize);
void coroutine_yield(struct scheduler_t *sched);
void coroutine_msleep(struct scheduler_t *sched, int ms);
void coroutine_usleep(struct scheduler_t *sched, int us);
void coroutine_nsleep(struct scheduler_t *sched, uint64_t ns);
```

## 图形系统

完整的 2D 图形渲染系统。

### Surface 抽象

Surface 是图形系统的核心对象，像素格式为 32 位预乘 ARGB：

```c
struct surface_t {
    uint32_t *pixels;
    int width;
    int height;
    int stride;
    int format;
};
```

### 创建与加载

- `surface_alloc(w, h)`：创建空白 Surface
- `surface_alloc_from_xfs()`：从 XFS 加载图像（支持 QOI、PNG、JPEG）
- `surface_alloc_from_buf()`：从内存缓冲加载
- `surface_alloc_qrcode()`：生成二维码 Surface

### 渲染操作

- **基础绘制**：`surface_fill()`（填充）、`surface_blit()`（位块传输）、`surface_text()`（文本）、`surface_icon()`（图标）
- **矢量形状**：`surface_shape_rectangle()`、`surface_shape_circle()`、`surface_shape_arc()`、`surface_shape_curve_to()` 等
- **变换**：`surface_shape_translate/scale/rotate/transform()`
- **特效**：`surface_effect_glass()`（毛玻璃）、`surface_effect_shadow()`（阴影）、`surface_effect_gradient()`（渐变）、`surface_effect_checkerboard()`（棋盘格）
- **滤镜**：`surface_filter_gray/sepia/invert/gamma/hue/saturate/brightness/contrast/opacity/blur()`

### 矩阵变换

2D 仿射变换矩阵：

```c
struct matrix2d_t {
    double a, b, c, d, e, f;
};
```

关键 API：`matrix2d_init_identity/translate/scale/rotate()`、`matrix2d_multiply/invert()`、`matrix2d_transform_point/distance/bounds/region()`

### 脏矩形优化

```c
struct dirtylist_t {
    struct list_head_t list;
};
```

关键 API：`dirtylist_alloc()`、`dirtylist_add()`、`dirtylist_clear()`、`dirtylist_merge()`、`dirtylist_clone()`

## KOBJ 虚拟文件系统

类似 Linux sysfs 的虚拟文件系统，用于访问设备状态和配置。

### 节点类型

```c
struct kobj_t {
    char *name;
    struct kobj_t *parent;
    struct list_head_t children;
    struct list_head_t sibling;

    /* 目录节点 */
    struct list_head_t list;

    /* 文件节点 */
    ssize_t (*read)(struct kobj_t *kobj, char *buf, size_t size);
    ssize_t (*write)(struct kobj_t *kobj, const char *buf, size_t size);
};
```

### 路径示例

- `/class/driver/clk-fixed/probe`：写入 JSON 动态探测设备
- `/class/memory/meminfo`：读取内存信息
- `/device/clk/clk.0/rate`：读取时钟频率

## 发布订阅系统

基于主题的发布订阅模式，用于事件驱动架构。

### API

```c
void psub_publish(const char *topic, void *data);
void psub_subscribe(const char *topic, void (*callback)(void *, void *), void *priv, int oneshot);
void psub_unsubscribe(const char *topic, void (*callback)(void *, void *), void *priv);
```

### 内置事件

- `device.add`：设备添加事件
- `device.remove`：设备移除事件
- `device.suspend`：设备挂起事件
- `device.resume`：设备恢复事件

## Initcall 机制

分级初始化机制，通过链接器段实现。

### Initcall 级别

```c
pure_initcall()      // 0 - 纯初始化（哈希表、设备/驱动列表）
machine_initcall()   // 1 - 机器初始化
core_initcall()      // 2 - 核心初始化
postcore_initcall()  // 3 - 核心后初始化
driver_initcall()    // 4 - 驱动初始化（最常用）
subsys_initcall()    // 5 - 子系统初始化
command_initcall()   // 6 - 命令初始化
server_initcall()    // 7 - 服务器初始化
wboxtest_initcall()  // 8 - 测试初始化
late_initcall()      // 9 - 延迟初始化
```

### 实现原理

```c
/* 链接器段定义 */
#define __define_initcall(level, fn) \
    static initcall_t __initcall_##fn \
    __attribute__((used)) \
    __attribute__((__section__("xstar_initcall_" #level))) = fn

/* 初始化执行 - 按级别 0-9 顺序 */
void do_initcalls(void)
{
    initcall_t *fn;
    for(level = 0; level < 10; level++)
        for(fn = __initcall_start[level]; fn < __initcall_end[level]; fn++)
            (*fn)();
}

/* 退出执行 - 按级别 9-0 逆序 */
void do_exitcalls(void)
{
    /* 逆序执行 exitcall */
}
```

## 设备树 (JSON)

使用 JSON 格式配置设备，比传统 DTS 更易读易写。

### 命名规则

```
"driver-name:id@address"
```

- `driver-name`：驱动名称
- `id`：设备 ID（可选）
- `address`：设备地址（可选）

### 配置示例

```json
{
    "fb-linux-sdl:0": {
        "width": 800,
        "height": 480
    },
    "i2c-gpio:0": {
        "sda-gpio": "gpio-v1-linux:2",
        "scl-gpio": "gpio-v1-linux:3",
        "delay-us": 5
    },
    "uart-linux:0": {
        "device": "/dev/ttyUSB0",
        "baud-rates": 115200,
        "status": "disabled"
    }
}
```

### 属性读取

通过 `dt_read_*` 函数读取设备树属性：

| 函数 | 返回类型 | 说明 |
|------|---------|------|
| `dt_read_string(n, name, def)` | `char *` | 读取字符串 |
| `dt_read_int(n, name, def)` | `int` | 读取整数 |
| `dt_read_long(n, name, def)` | `long long` | 读取长整数 |
| `dt_read_bool(n, name, def)` | `int` | 读取布尔值 |
| `dt_read_double(n, name, def)` | `double` | 读取双精度浮点 |
| `dt_read_object(n, name)` | `struct dtnode_t` | 读取子对象 |

### 状态控制

设置 `"status": "disabled"` 可跳过设备探测。

### 设备引用

通过 `"driver-name:id"` 格式引用其他设备：

```json
{
    "led-gpio:0": {
        "gpio": "gpio-v1-linux:10",
        "active-low": true
    }
}
```

## 工具库 (LibX)

LibX 提供通用的算法、数据结构、加密、编码等工具函数，是 XOS 之上的基础库。

### 数据结构

| 模块 | 说明 | 关键 API |
|------|------|---------|
| 双向链表 | `list.h`（纯头文件） | `init_list_head()`, `list_add/del/splice()`, `list_for_each_entry()` |
| 哈希链表 | `list.h`（纯头文件） | `struct hlist_head_t`, `struct hlist_node_t` |
| 单向链表 | `slist.c/h` | 单指针链表 |
| FIFO 环形缓冲 | `fifo.c/h` | 无锁环形缓冲区 |
| 队列 | `queue.c/h` | 通用队列 |
| 哈希表 | `hmap.c/h` | 键值对哈希表 |
| 红黑树 | `rbtree.c/h` | 自平衡二叉搜索树 |
| LRU 缓存 | `lru.c/h` | 最近最少使用缓存 |
| 动态字符串 | `ds.c/h` | `struct ds_t`，支持追加、插入、删除、查找、替换 |
| KOBJ | `kobj.c/h` | 层次化虚拟文件系统节点 |
| 设备树 | `dtree.c/h` | JSON 设备树解析器 |
| 初始化调用 | `initcall.c/h` | 链接器段 init/exit 注册（10 级） |

### 加密/安全

| 模块 | 说明 |
|------|------|
| AES-128 | `aes128.c/h` |
| AES-256 | `aes256.c/h` |
| RC4 | `rc4.c/h` |
| ECDSA-256 | `ecdsa256.c/h` |
| SHA-1 | `sha1.c/h` |
| SHA-256 | `sha256.c/h` |
| VM 加密 | `vmcrypt.c/h` |
| 里德所罗门 | `rs.c/h`（前向纠错编码） |

### 信号处理/算法

| 模块 | 说明 |
|------|------|
| FFT | `fft.c/h`（快速傅里叶变换） |
| 双二阶滤波器 | `biquad.c/h`（二阶 IIR 滤波器） |
| 卡尔曼滤波 | `kalman.c/h` |
| EWMA | `ewma.c/h`（指数加权移动平均） |
| 中值滤波 | `median.c/h` |
| 均值滤波 | `mean.c/h` |
| 时序滤波 | `tsfilter.c/h` |
| 按键滤波 | `keyfilter.c/h`（输入去抖） |
| 余弦查找表 | `costab.c/h` |
| 复数运算 | `complex.c/h` |
| 弹簧动画 | `spring.c/h` |
| 退避算法 | `backoff.c/h`（指数退避） |
| 呼吸灯 | `breathing.c/h` |
| 缓动函数 | `easing.c/h` |
| 窗函数 | `winfunc.c/h`（Hamming、Hanning 等） |

### 编码/压缩

| 模块 | 说明 |
|------|------|
| Base64 | `base64.c/h` |
| JSON | `json.c/h` |
| URI | `uri.c/h` |
| QR 码生成 | `qrcgen.c/h` |
| 交织器 | `interleaver.c/h` |
| 十六进制转储 | `hexdump.c/h` |
| CRC-8/16/32 | `crc8.c/h`、`crc16.c/h`、`crc32.c/h` |
| 字符串哈希 | `shash.h`（纯头文件） |
| 字符集 | `charset.c/h` |
| YUV 转换 | `yuv.c/h` |

### 字符串/文件工具

| 模块 | 说明 |
|------|------|
| 路径操作 | `path.c/h` |
| UUID 生成 | `uuid.c/h` |
| 数据库 | `db.c/h` |
| 进程列表 | `ps.c/h` |
| 内存池 | `mm.c/h` |
| 整数平方根 | `sqrti.c/h` |
| 非对齐访问 | `unaligned.h`（纯头文件） |
| 字节序转换 | `byteorder.h`（纯头文件，be16/le16/be32/le32） |
| BCD 编码 | `bcd.h`（纯头文件） |
| 排序列表 | `lsort.c/h` |
| 分贝转换 | `db.c/h` |
| 内核时间 | `ktime.h`（纯头文件） |
| 核心定义 | `xdef.h`（纯头文件，`NULL`/`TRUE`/`FALSE`/`container_of`/`XMIN`/`XMAX`） |

## 构建系统

### Kconfig 配置

使用 Kconfig 系统进行配置管理：

```bash
make <project>/xstar.defconfig    # 应用项目默认配置
make menuconfig                    # 交互式配置菜单
```

配置保存在 `.config` 文件中，生成 `.config.h` 供源码使用。

### Kbuild 编译

使用 Kbuild 风格的构建系统：

```makefile
obj-y += core.o                              # 始终编译
obj-$(CONFIG_DRV_CLK_FIXED) += clk-fixed.o    # 条件编译
subdirs-y += adc                              # 递归子目录
```

### 项目结构

每个项目位于 `projects/` 目录下，包含：

- `xstar.defconfig`：项目默认配置
- `xstarcfg.h`：项目特定的类型定义和平台头文件
- `main.c`：平台特定的入口点
- `linux/` 或 `baremetal/`：平台实现代码
- `romdisk/`：只读文件系统（含 `dtree/default.json` 设备树）

### 构建输出

编译输出位于 `projects/<project>/output/`，最终生成 `xstar` 可执行文件。

## 数据流

### 设备探测流程

```
系统启动 → 解析设备树 (JSON) → 匹配驱动 → 调用 probe() → 创建设备实例 → 注册到系统
```

### 协程调度流程

```
协程启动 → 执行用户函数 → 调用 yield() → 保存上下文 → 切换到调度器 → 选择下一个协程 → 恢复上下文
```

### 图形渲染流程

```
应用调用 → 图形操作 → 记录脏矩形 → 渲染到 surface → G2D/软件渲染 → 更新到帧缓冲
```

### 发布订阅流程

```
发布事件 → 查找订阅者 → 遍历回调列表 → 执行回调函数 → （可选）取消 oneshot 订阅
```

## 扩展性

### 添加新驱动

1. 实现 `driver_t` 结构
2. 实现 `probe/remove/suspend/resume` 回调
3. 使用 `driver_initcall` 注册驱动
4. 在设备树中添加设备配置

### 添加新命令

1. 实现 `command_t` 结构
2. 实现 `exec` 回调函数
3. 使用 `command_initcall` 注册命令
4. 添加帮助文档

### 添加新内核子系统

1. 定义子系统接口
2. 实现核心功能
3. 提供对外 API
4. 使用适当的 initcall 级别注册
