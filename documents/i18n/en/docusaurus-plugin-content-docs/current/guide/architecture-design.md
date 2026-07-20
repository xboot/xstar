# Architecture Design

This document describes the XSTAR system architecture and core component design, focusing on "what/why".

## Table of Contents

- [Overall Architecture](#overall-architecture)
- [Boot Flow](#boot-flow)
- [Cross-Platform Abstraction Layer (XOS)](#cross-platform-abstraction-layer-xos)
- [Device Driver Framework](#device-driver-framework)
- [Kernel Subsystems](#kernel-subsystems)
- [Coroutine System](#coroutine-system)
- [Graphics System](#graphics-system)
- [KOBJ Virtual File System](#kobj-virtual-file-system)
- [Publish-Subscribe System](#publish-subscribe-system)
- [Initcall Mechanism](#initcall-mechanism)
- [Device Tree (JSON)](#device-tree-json)
- [Utility Library (LibX)](#utility-library-libx)
- [Data Flow](#data-flow)

## Overall Architecture

XSTAR adopts a layered architecture, from bottom to top:

```
┌──────────────────────────────────────────────────────────────────┐
│                       Application Layer                          │
├──────────────────────────────────────────────────────────────────┤
│              Kernel Subsystems                                   │
│  Audio | Command | Core | Font | Graphic | Shell | Time          │
│  Vision | Window | XFS                                           │
├──────────────────────────────────────────────────────────────────┤
│               Device Driver Framework                            │
│          Driver | Device | Class | KOBJ | DTREE (JSON)           │
├──────────────────────────────────────────────────────────────────┤
│                 Platform Abstraction Layer (XOS)                 │
│  Memory | DMA | I/O | File | Thread | Mutex | Semaphore          │
│  Coroutine | PM | Stdio                                          │
├──────────────────────────────────────────────────────────────────┤
│                  Utility Library (LibX)                          │
│  Algorithm | Data Structure | Crypto | JSON | DTREE | Encoding   │
├──────────────────────────────────────────────────────────────────┤
│          Hardware Platforms                                      │
│  ARM32/64 | RISC-V32/64 | x64 (Baremetal/Linux/FreeRTOS)         │
└──────────────────────────────────────────────────────────────────┘
```

### Layer Descriptions

1. **Hardware Platform Layer**: Provides basic hardware support, including CPU, memory, peripherals, etc.
2. **LibX Utility Library**: Provides general-purpose algorithms, data structures, crypto, encoding, and other utility functions
3. **XOS Abstraction Layer**: Shields lower-layer differences, providing a unified syscall interface
4. **Device Driver Framework**: Manages device drivers and device instances, implementing automatic driver probing and registration
5. **Kernel Subsystems**: Provides core functionality such as audio, graphics, command, shell, time, vision, window, file system, etc.
6. **Application Layer**: Upper-layer applications built on top of kernel subsystems

## Boot Flow

The system entry point is `xstar_init()` (`xstar/xstar.c`). The platform-specific `main()` calls `xstar_init(&env, json)` to complete the entire system initialization:

```
main() -> xstar_init()
  ├── xos_environ_init(env)         Install platform abstraction function table
  ├── do_initcalls()                Execute all initcalls by level (0->9)
  ├── do_init_romdisk()             Register romdisk block device
  ├── do_init_dtree(dtree)          Parse device tree JSON, probe all devices
  ├── do_init_wallclock()           Calibrate wall clock from RTC device
  ├── do_init_memory()              Register memory info KOBJ
  ├── do_init_logger()              Register logger control KOBJ
  ├── do_init_version()             Register version info KOBJ
  ├── do_init_copyright()           Register copyright verification KOBJ
  ├── do_init_random()              Initialize random number generator
  ├── do_init_feature()             Detect coroutine/thread support features
  ├── do_init_font()                Load TrueType font
  ├── do_init_setting()             Initialize persistent setting system
  ├── do_init_final()               Execute final-level initcalls
  └── do_show_logo()                Display boot logo on framebuffer
```

Key steps:

- **`xos_environ_init(env)`**: Installs platform-provided function pointers into the global `__xos_environ` struct, only overwriting non-NULL entries
- **`do_initcalls()`**: Executes initialization functions in linker sections sequentially by level 0-9; driver registration, subsystem initialization, and command registration all happen in this phase
- **`do_init_dtree(dtree)`**: Parses the device tree JSON, matches a driver for each device node and calls `probe()` to create device instances
- **`do_init_wallclock()`**: Iterates registered RTC devices, reads valid time to calibrate the wall clock (only adopted when the RTC time is reasonable)
- **`do_init_final()`**: Executes `final_initcall`-level initialization functions, used for last-stage deferred initialization

## Cross-Platform Abstraction Layer (XOS)

XOS is the core abstraction layer of XSTAR, shielding differences across platforms and runtime environments through a function pointer table.

### Interface Abstraction

```c
struct xos_environ_t {
    /* Memory management */
    void *(*malloc)(size_t size);
    void (*free)(void *ptr);
    void *(*realloc)(void *ptr, size_t size);

    /* DMA operations */
    int (*dma_alloc)(void *addr, size_t size, uint64_t pa);
    int (*dma_free)(void *addr, size_t size);

    /* IO operations */
    void (*write8)(io_addr_t addr, uint8_t value);
    void (*write16)(io_addr_t addr, uint16_t value);
    void (*write32)(io_addr_t addr, uint32_t value);
    uint8_t (*read8)(io_addr_t addr);
    uint16_t (*read16)(io_addr_t addr);
    uint32_t (*read32)(io_addr_t addr);

    /* File system */
    void *(*fopen)(const char *path, const char *mode);
    int (*fclose)(void *file);
    size_t (*fread)(void *ptr, size_t size, size_t nmemb, void *file);
    size_t (*fwrite)(const void *ptr, size_t size, size_t nmemb, void *file);

    /* Coroutine */
    void (*coroutine_make)(void *stack, size_t size, void (*func)(struct co_transfer_t));
    struct co_transfer_t (*coroutine_jump)(void *fctx, void *priv);

    /* Thread */
    void *(*thread_create)(const char *name, void (*func)(void *), void *data, int stksz);
    void (*thread_destroy)(void *thread);
    void (*thread_wait)(void *thread);
    void (*thread_sleep)(uint64_t ns);

    /* Mutex */
    int (*mutex_init)(void *mutex);
    int (*mutex_lock)(void *mutex);
    int (*mutex_unlock)(void *mutex);
    int (*mutex_exit)(void *mutex);

    /* Semaphore */
    int (*semaphore_init)(void *sem, uint32_t count);
    int (*semaphore_wait)(void *sem, uint32_t timeout);
    int (*semaphore_post)(void *sem);
    int (*semaphore_exit)(void *sem);

    /* Power management */
    void (*shutdown)(void);
    void (*reboot)(void);
    void (*standby)(void);
};
```

### API Categories

XOS provides platform-related operations via `xos_environ_t`, and also directly provides a large number of platform-independent portable APIs:

| Category | Key Functions |
|----------|---------------|
| **Memory Management** | `xos_mem_malloc`, `xos_mem_free`, `xos_mem_realloc`, `xos_mem_calloc`, `xos_mem_memalign`, `xos_mem_meminfo` |
| **DMA Operations** | `xos_dma_alloc_coherent`, `xos_dma_free_coherent`, `xos_dma_alloc_noncoherent`, `xos_dma_sync` |
| **Hardware I/O** | `xos_io_read8/16/32/64`, `xos_io_write8/16/32/64`, `xos_io_clrbits/setbits/clrsetbits` |
| **Standard I/O** | `xos_stdio_read`, `xos_stdio_write` |
| **File System** | `xos_file_open/close/read/write/seek`, `xos_file_mkdir/remove/walk`, `xos_file_isdir/isfile` |
| **Coroutine** | `xos_coroutine_make`, `xos_coroutine_jump` |
| **Thread** | `xos_thread_create/destroy/wait/sleep` |
| **Mutex** | `xos_mutex_init/exit/lock/unlock/trylock` |
| **Semaphore** | `xos_semaphore_init/exit/wait/post` |
| **Power Management** | `xos_pm_shutdown/reboot/standby` |
| **String Operations** | `xos_strcmp/strcpy/strcat/strlen/strstr/strdup/strtok`, etc. |
| **Formatting** | `xos_sprintf/snprintf/printf/sscanf`, etc. |
| **Numeric Conversion** | `xos_strtol/strtoll/strtod/atoi/atol`, etc. |
| **Sort & Search** | `xos_qsort`, `xos_bsearch` |
| **Random Numbers** | `xos_srand/rand/random_int/random_float` |

### Platform Implementations

Each platform provides a corresponding `xos_environ_t` implementation:

- `xos-linux.c`: Linux/SDL platform implementation
- `xos-baremetal-arm64.c`: ARM64 bare-metal implementation
- `xos-baremetal-arm32.c`: ARM32 bare-metal implementation
- `xos-baremetal-riscv64.c`: RISC-V64 bare-metal implementation
- `xos-baremetal-riscv32.c`: RISC-V32 bare-metal implementation
- `xos-freertos.c`: FreeRTOS platform implementation
- `xos-windows.c`: Windows platform implementation

## Device Driver Framework

The device driver framework adopts a driver/device separation design, implementing automatic driver probing and registration.

### Core Data Structures

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

### Device Types

The system defines 50+ device types (`enum device_type_t`):

| Category | Types |
|----------|-------|
| Clock | `CLK`, `CLOCKEVENT`, `CLOCKSOURCE` |
| Storage | `BLOCK`, `NVMEM`, `SDHCI` |
| Display | `FRAMEBUFFER`, `G2D`, `CONSOLE` |
| Audio | `AUDIOCAPTURE`, `AUDIOPLAYBACK`, `BUZZER` |
| Input | `INPUT`, `CAMERA` |
| Communication | `I2C`, `SPI`, `UART`, `NET`, `ATNET`, `GNSS` |
| GPIO | `GPIOCHIP`, `IRQCHIP`, `RESETCHIP` |
| Sensors | `ADC`, `COMPASS`, `GMETER`, `GYROSCOPE`, `HYGROMETER`, `LIGHT`, `OXIMETER`, `PRESSURE`, `PROXIMITY`, `THERMOMETER` |
| Output | `LED`, `LEDSTRIP`, `LEDTRIGGER`, `DAC`, `PWM`, `SERVO`, `MOTOR`, `STEPPER`, `VIBRATOR` |
| Power | `BATTERY`, `REGULATOR`, `RNG`, `RTC` |
| Others | `ATOMIC`, `DMA`, `LIMITER`, `PRINTER`, `SPINLOCK`, `WATCHDOG` |

### Driver Registration

Drivers are automatically registered via the `driver_initcall` macro:

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

### Driver Lookup

Drivers are stored in a hash table (521 buckets, using the `shash()` hash function), with lookup efficiency close to O(1):

- `register_driver(drv)`: Creates a KOBJ directory `/class/driver/<name>/`, adds a `probe` write entry, and inserts into the hash table
- `unregister_driver(drv)`: Removes from the hash table and KOBJ tree
- `search_driver(name)`: Looks up a driver by name in the hash table

### Device Registration

Devices are managed through a triple index:

- Global linked list `__device_list`
- Per-type linked list `__device_head[type]`
- Name-based hash table

`register_device(dev)` adds the device to all three indexes and publishes a device-add event via `psub_publish("device.add", dev)`.

### Device Probing

At system boot, `probe_device()` parses the device tree JSON, automatically matching and probing a driver for each node:

```
Parse JSON -> extract key "driver-name:id@addr" -> search_driver(name)
-> call drv->probe(drv, n) -> create device instance -> register to system
```

For the complete driver development template (probe/remove/suspend/resume, device tree configuration, Kbuild), see [Development Guide - Driver Development](./development-guide#driver-development).

## Kernel Subsystems

The XSTAR kernel contains the following subsystems. For detailed API documentation of each subsystem, see the [Subsystem Docs](../subsys/dtree/device-tree):

| Subsystem | Description | Docs |
|-----------|-------------|------|
| **Audio** | Full audio processing pipeline: Source -> Mixer -> Effect -> Sink | [Audio Overview](../subsys/audio/overview) |
| **Command** | Unified command interface, 32+ built-in commands | [Built-in Commands](../command/help) |
| **Core** | Coroutine, Logger, Profiler, Setting, ThChannel, ThWorker, CoChannel, PSub and other core utilities | See links below |
| **Font** | 4 font style management, TrueType/CFF parsing | [Font System](../subsys/font/font-system) |
| **Graphic** | 2D graphics rendering: Surface, shapes, transforms, effects, filters | [Graphics - Surface](../subsys/graphic/surface) |
| **Shell** | Interactive shell, command completion, history, working directory | [Shell](../subsys/shell) |
| **Time** | Red-black tree high-precision timer, wall clock, delay | [Timer](../subsys/timer) |
| **Vision** | Image processing algorithms (grayscale/RGB888): morphology, threshold, filtering, drawing | [Vision Core Types](../subsys/vision/core-types) |
| **Window** | Window management, event handling, dirty rectangles, backlight | [Window](../subsys/window) |
| **XFS** | Virtual file system, multiple mount points, pluggable archivers | [File System](../subsys/xfs) |

Independent docs for each tool under the Core subsystem:

- Coroutine: [Coroutine](../subsys/coroutine), [CoChannel](../subsys/coroutine/cochannel)
- Thread: [ThChannel](../subsys/thread/thchannel), [ThWorker](../subsys/thread/thworker)
- Debug: [Logger](../subsys/debug/logger), [Profiler](../subsys/debug/profiler)
- Persistence: [Setting](../subsys/setting)
- Publish-Subscribe: [PSub](../subsys/psub)

## Coroutine System

XSTAR provides assembly-level coroutine implementations, supporting multiple architectures.

### Coroutine Context

Each architecture provides complete context save and restore:

- **ARM32**: Saves R0-R15, CPSR, etc.
- **ARM64**: Saves X0-X30, SP, PC, PSTATE, etc.
- **RISC-V32**: Saves x0-x31, SP, PC, etc.
- **RISC-V64**: Saves x0-x31, SP, PC, etc.
- **x64**: Saves RAX-R15, RSP, RIP, etc.

### Coroutine Scheduler

```c
struct scheduler_t {
    struct list_head_t runlist;
    struct coroutine_t * running;
    struct mutex_t * mutex;
};
```

### Coroutine API

```c
void coroutine_start(struct scheduler_t *sched, void (*func)(void *), void *data, size_t ssize);
void coroutine_yield(struct scheduler_t *sched);
void coroutine_msleep(struct scheduler_t *sched, int ms);
void coroutine_usleep(struct scheduler_t *sched, int us);
void coroutine_nsleep(struct scheduler_t *sched, uint64_t ns);
```

## Graphics System

A complete 2D graphics rendering system, with `surface_t` (32-bit premultiplied ARGB) as the core object.

- **Create & Load**: `surface_alloc(w, h)`, `surface_alloc_from_xfs()` (QOI/PNG/JPEG), `surface_alloc_from_buf()`, `surface_alloc_qrcode()`
- **Rendering**: Fill, bit-block transfer, text, icons, vector shapes (rectangle/circle/arc/curve), affine transforms
- **Effects**: Frosted glass, shadow, gradient, checkerboard
- **Filters**: Grayscale, sepia, invert, gamma, hue, saturation, brightness, contrast, opacity, blur

The graphics system and vision system can convert to each other: `vision_apply_surface()` / `surface_apply_vision()`.

For detailed APIs on Surface, shape drawing, transform matrices, dirty rectangles, filters, etc., see the [Graphics Subsystem Docs](../subsys/graphic/surface).

## KOBJ Virtual File System

A virtual file system similar to Linux sysfs, used to access device status and configuration.

### Node Types

```c
struct kobj_t {
    char *name;
    struct kobj_t *parent;
    struct list_head_t children;
    struct list_head_t sibling;

    /* Directory node */
    struct list_head_t list;

    /* File node */
    ssize_t (*read)(struct kobj_t *kobj, char *buf, size_t size);
    ssize_t (*write)(struct kobj_t *kobj, const char *buf, size_t size);
};
```

### Path Examples

- `/class/driver/clk-fixed/probe`: Write JSON to dynamically probe a device
- `/class/memory/meminfo`: Read memory info
- `/device/clk/clk.0/rate`: Read clock frequency

## Publish-Subscribe System

A topic-based publish-subscribe pattern for event-driven architecture.

### API

```c
void psub_publish(const char *topic, void *data);
void psub_subscribe(const char *topic, void (*callback)(void *, void *), void *priv, int oneshot);
void psub_unsubscribe(const char *topic, void (*callback)(void *, void *), void *priv);
```

### Built-in Events

- `device.add`: Device added event
- `device.remove`: Device removed event
- `device.suspend`: Device suspended event
- `device.resume`: Device resumed event

For the complete publish-subscribe API, see the [Publish-Subscribe Docs](../subsys/psub).

## Initcall Mechanism

A leveled initialization mechanism implemented via linker sections.

### Initcall Levels

```c
pure_initcall()      // 0 - pure init (hash tables, device/driver lists)
machine_initcall()   // 1 - machine init
core_initcall()      // 2 - core init
postcore_initcall()  // 3 - post-core init
driver_initcall()    // 4 - driver init (most common)
subsys_initcall()    // 5 - subsystem init
command_initcall()   // 6 - command init
server_initcall()    // 7 - server init
wboxtest_initcall()  // 8 - test init
late_initcall()      // 9 - late init
final_initcall()     // final - last-stage init (executed separately by do_init_final())
```

### Implementation Principle

```c
/* Linker section definition */
#define __define_initcall(level, fn) \
    static initcall_t __initcall_##fn \
    __attribute__((used)) \
    __attribute__((__section__("xstar_initcall_" #level))) = fn

/* Initialization execution - by level 0-9 in order */
void do_initcalls(void)
{
    initcall_t *fn;
    for(level = 0; level < 10; level++)
        for(fn = __initcall_start[level]; fn < __initcall_end[level]; fn++)
            (*fn)();
}

/* Exit execution - by level 9-0 in reverse order */
void do_exitcalls(void)
{
    /* Execute exitcalls in reverse order */
}
```

## Device Tree (JSON)

Uses JSON format to configure devices, more readable and writable than traditional DTS.

### Naming Rules

```
"driver-name:id@address"
```

- `driver-name`: Driver name
- `id`: Device ID (optional)
- `address`: Device address (optional)

### Configuration Example

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

Set `"status": "disabled"` to skip device probing. Reference other devices via the `"driver-name:id"` format.

For property read functions (`dt_read_string/int/long/bool/double/object`) and full usage, see the [Device Tree Docs](../subsys/dtree/device-tree).

## Utility Library (LibX)

LibX provides general-purpose algorithms, data structures, crypto, encoding, and other utility functions. It is the base library above XOS and is currently compiled unconditionally.

| Category | Module Examples |
|----------|-----------------|
| **Data Structures** | Doubly linked list, hash list, singly linked list, FIFO, queue, hash map, red-black tree, LRU, dynamic string, KOBJ, device tree, initcall |
| **Crypto/Security** | AES-128/256, RC4, ECDSA-256, SHA-1/256, VM encryption, Reed-Solomon |
| **Signal/Algorithms** | FFT, biquad filter, Kalman, EWMA, median/mean filter, time-series/key filter, spring animation, backoff, breathing light, easing, window function |
| **Encoding/Compression** | Base64, JSON, URI, QR code, interleaver, hex dump, CRC-8/16/32, charset, YUV conversion |
| **String/Utils** | Path, UUID, memory pool, integer square root, unaligned access, byte order, BCD, sorted list, decibel, kernel time |

For detailed APIs of each module, see the [Library Docs](../libx/xdef).

## Data Flow

### Device Probing Flow

```
System boot -> parse device tree (JSON) -> match driver -> call probe() -> create device instance -> register to system
```

### Coroutine Scheduling Flow

```
Coroutine start -> execute user function -> call yield() -> save context -> switch to scheduler -> select next coroutine -> restore context
```

### Graphics Rendering Flow

```
Application call -> graphics operation -> record dirty rectangle -> render to surface -> G2D/software rendering -> update to framebuffer
```

### Publish-Subscribe Flow

```
Publish event -> find subscribers -> iterate callback list -> execute callback function -> (optional) cancel oneshot subscription
```
