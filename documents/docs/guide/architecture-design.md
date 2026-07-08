# 架构设计

本文档说明 XSTAR 的系统架构以及核心组件设计，聚焦"是什么/为什么"。

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
- [数据流](#数据流)

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
  ├── do_init_dtree(dtree)          解析设备树 JSON，探测所有设备
  ├── do_init_wallclock()           从 RTC 设备校准墙钟时间
  ├── do_init_memory()              注册内存信息 KOBJ
  ├── do_init_logger()              注册日志控制 KOBJ
  ├── do_init_version()             注册版本信息 KOBJ
  ├── do_init_copyright()           注册版权验证 KOBJ
  ├── do_init_random()              初始化随机数生成器
  ├── do_init_feature()             检测协程/线程支持特性
  ├── do_init_font()                加载 TrueType 字体
  ├── do_init_setting()             初始化持久化设置系统
  ├── do_init_final()               执行 final 级别 initcall
  └── do_show_logo()                在帧缓冲上显示启动 Logo
```

关键步骤说明：

- **`xos_environ_init(env)`**：将平台提供的函数指针安装到全局 `__xos_environ` 结构体中，仅覆盖非 NULL 的条目
- **`do_initcalls()`**：按级别 0-9 顺序执行链接器段中的初始化函数，驱动注册、子系统初始化、命令注册都在此阶段完成
- **`do_init_dtree(dtree)`**：解析设备树 JSON，对每个设备节点匹配驱动并调用 `probe()`，创建设备实例
- **`do_init_wallclock()`**：遍历已注册的 RTC 设备，读取有效时间校准墙钟（仅当 RTC 时间合理时采纳）
- **`do_init_final()`**：执行 `final_initcall` 级别的初始化函数，用于最末阶段的延迟初始化

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

系统定义了 50+ 设备类型（`enum device_type_t`）：

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

完整的驱动开发模板（probe/remove/suspend/resume、设备树配置、Kbuild）见[开发指南 - 驱动开发](./development-guide#驱动开发)。

## 内核子系统

XSTAR 内核包含以下子系统，各子系统的详细 API 文档见[子系统文档](../subsys/dtree/device-tree)：

| 子系统 | 说明 | 文档 |
|------|------|------|
| **Audio** | 完整音频处理链路：Source → Mixer → Effect → Sink | [音频概述](../subsys/audio/overview) |
| **Command** | 统一命令接口，32+ 内置命令 | [内置命令](../command/help) |
| **Core** | 协程、Logger、Profiler、Setting、ThChannel、ThWorker、CoChannel、PSub 等核心工具 | 见下文各链接 |
| **Font** | 4 种字型管理，TrueType/CFF 解析 | [字体系统](../subsys/font/font-system) |
| **Graphic** | 2D 图形渲染：Surface、形状、变换、特效、滤镜 | [图形 - Surface](../subsys/graphic/Surface) |
| **Shell** | 交互式 Shell，命令补全、历史记录、工作目录 | [Shell](../subsys/shell) |
| **Time** | 红黑树高精度定时器、墙钟时间、延时 | [定时器](../subsys/timer) |
| **Vision** | 图像处理算法（灰度/RGB888）：形态学、阈值、滤波、绘制 | [视觉核心类型](../subsys/vision/core-types) |
| **Window** | 窗口管理、事件处理、脏矩形、背光 | [窗口](../subsys/window) |
| **XFS** | 虚拟文件系统，多挂载点，可插拔归档器 | [文件系统](../subsys/xfs) |

Core 子系统下的各工具独立文档：

- 协程：[协程](../subsys/coroutine)、[CoChannel](../subsys/coroutine/cochannel)
- 线程：[ThChannel](../subsys/thread/thchannel)、[ThWorker](../subsys/thread/thworker)
- 调试：[Logger](../subsys/debug/logger)、[Profiler](../subsys/debug/profiler)
- 持久化：[Setting](../subsys/setting)
- 发布订阅：[PSub](../subsys/psub)

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

完整的 2D 图形渲染系统，核心对象为 `surface_t`（32 位预乘 ARGB）。

- **创建与加载**：`surface_alloc(w, h)`、`surface_alloc_from_xfs()`（QOI/PNG/JPEG）、`surface_alloc_from_buf()`、`surface_alloc_qrcode()`
- **渲染**：填充、位块传输、文本、图标、矢量形状（矩形/圆/弧/曲线）、仿射变换
- **特效**：毛玻璃、阴影、渐变、棋盘格
- **滤镜**：灰度、怀旧、反转、伽马、色相、饱和度、亮度、对比度、不透明度、模糊

图形系统与视觉系统可互相转换：`vision_apply_surface()` / `surface_apply_vision()`。

详细的 Surface、形状绘制、变换矩阵、脏矩形、滤镜等 API 见[图形子系统文档](../subsys/graphic/Surface)。

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

完整的发布订阅 API 见[发布订阅文档](../subsys/psub)。

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
final_initcall()     // final - 最末阶段初始化（由 do_init_final() 单独执行）
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
    }
}
```

设置 `"status": "disabled"` 可跳过设备探测。通过 `"driver-name:id"` 格式引用其他设备。

属性读取函数（`dt_read_string/int/long/bool/double/object`）及完整用法见[设备树文档](../subsys/dtree/device-tree)。

## 工具库 (LibX)

LibX 提供通用的算法、数据结构、加密、编码等工具函数，是 XOS 之上的基础库，目前无条件编译。

| 分类 | 模块示例 |
|------|---------|
| **数据结构** | 双向链表、哈希链表、单向链表、FIFO、队列、哈希表、红黑树、LRU、动态字符串、KOBJ、设备树、initcall |
| **加密/安全** | AES-128/256、RC4、ECDSA-256、SHA-1/256、VM 加密、里德所罗门 |
| **信号/算法** | FFT、双二阶滤波器、卡尔曼、EWMA、中值/均值滤波、时序/按键滤波、弹簧动画、退避、呼吸灯、缓动、窗函数 |
| **编码/压缩** | Base64、JSON、URI、QR 码、交织器、十六进制转储、CRC-8/16/32、字符集、YUV 转换 |
| **字符串/工具** | 路径、UUID、内存池、整数平方根、非对齐访问、字节序、BCD、排序列表、分贝、内核时间 |

各模块的详细 API 见[基础库文档](../libx/xdef)。

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

