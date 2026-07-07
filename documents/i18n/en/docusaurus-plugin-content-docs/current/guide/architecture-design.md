# XSTAR Architecture Design

This document describes in detail the system architecture and core component design of XSTAR.

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
- [Build System](#build-system)

## Overall Architecture

XSTAR adopts a layered architecture, from bottom to top:

```
┌──────────────────────────────────────────────────────────────────┐
│                        Application Layer                          │
├──────────────────────────────────────────────────────────────────┤
│                       Kernel Subsystems                           │
│  Audio | Command | Core | Font | Graphic | Shell | Time          │
│  Vision | Window | XFS                                            │
├──────────────────────────────────────────────────────────────────┤
│                       Driver Framework                            │
│          Driver | Device | Class | KOBJ | DTREE (JSON)            │
├──────────────────────────────────────────────────────────────────┤
│                  Platform Abstraction (XOS)                       │
│  Memory | DMA | I/O | File | Thread | Mutex | Semaphore          │
│  Coroutine | PM | Stdio                                           │
├──────────────────────────────────────────────────────────────────┤
│                      Utility Library (LibX)                       │
│  Algorithm | Data Structure | Crypto | JSON | DTREE | Encoding   │
├──────────────────────────────────────────────────────────────────┤
│                      Hardware Platforms                           │
│  ARM32/64 | RISC-V32/64 | x64 (Baremetal/Linux/FreeRTOS)         │
└──────────────────────────────────────────────────────────────────┘
```

### Layer Description

1. **Hardware Platform Layer**: Provides foundational hardware support, including CPU, memory, peripherals, etc.
2. **LibX Utility Library**: Provides common utility functions for algorithms, data structures, cryptography, encoding, etc.
3. **XOS Abstraction Layer**: Shields lower-layer differences and provides a unified system call interface.
4. **Device Driver Framework**: Manages device drivers and device instances, implementing automatic driver probing and registration.
5. **Kernel Subsystems**: Provides core functionality such as audio, graphics, command, shell, time, vision, window, and file system.
6. **Application Layer**: Upper-layer applications built on top of the kernel subsystems.

## Boot Flow

The system entry point is `xstar_init()` (`xstar/xstar.c`). The platform-specific `main()` calls `xstar_init(&env, json)` to complete the entire system initialization:

```
main() → xstar_init()
  ├── xos_environ_init(env)         Install the platform abstraction function table
  ├── do_initcalls()                Execute all initcalls by level (0→9)
  ├── do_init_romdisk()             Register the romdisk block device
  ├── do_init_dtree(json)           Parse the device tree JSON and probe all devices
  ├── do_init_memory()              Register the memory info KOBJ
  ├── do_init_logger()              Register the log control KOBJ
  ├── do_init_version()             Register the version info KOBJ
  ├── do_init_copyright()           Register the copyright verification KOBJ
  ├── do_init_random()              Initialize the random number generator
  ├── do_init_feature()             Detect coroutine/thread support features
  ├── do_init_font()                Load the TrueType font
  ├── do_init_setting()             Initialize the persistent settings system
  └── do_show_logo()                Display the boot logo on the framebuffer
```

Key steps:

- **`xos_environ_init(env)`**: Installs the function pointers provided by the platform into the global `__xos_environ` struct, only overwriting non-NULL entries.
- **`do_initcalls()`**: Executes the initialization functions in the linker sections in level order 0-9. Driver registration, subsystem initialization, and command registration all happen in this phase.
- **`do_init_dtree(json)`**: Parses the device tree JSON, matches a driver for each device node, calls `probe()`, and creates device instances.

## Cross-Platform Abstraction Layer (XOS)

XOS is the core abstraction layer of XSTAR. It uses a function pointer table to shield the differences between platforms and runtime environments.

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

XOS provides platform-related operations through `xos_environ_t`, and also directly provides a large number of platform-independent portable APIs:

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

The device driver framework uses a driver/device separation design, implementing automatic driver probing and registration.

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

The system defines 51 device types (`enum device_type_t`):

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
| Other | `ATOMIC`, `DMA`, `LIMITER`, `PRINTER`, `SPINLOCK`, `WATCHDOG` |

### Driver Registration

Drivers are registered automatically via the `driver_initcall` macro:

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

- `register_driver(drv)`: Creates the KOBJ directory `/class/driver/<name>/`, adds a `probe` write entry, and inserts into the hash table.
- `unregister_driver(drv)`: Removes from the hash table and the KOBJ tree.
- `search_driver(name)`: Looks up a driver by name in the hash table.

### Device Registration

Devices are managed via a triple index:

- Global list `__device_list`
- Per-type list `__device_head[type]`
- Per-name hash table

`register_device(dev)` adds the device to the triple index and publishes a device-added event via `psub_publish("device.add", dev)`.

### Device Probing

At system startup, `probe_device()` parses the device tree JSON, automatically matches and probes a driver for each node:

```
Parse JSON → extract key "driver-name:id@addr" → search_driver(name)
→ call drv->probe(drv, n) → create device instance → register into the system
```

## Kernel Subsystems

### Audio System (Audio)

Complete audio processing chain:

```
Source (audio source) → Mixer (mixer) → Effect (audio effects) → Sink (output device)
```

- **Source**: Audio input abstraction, supporting files (WAV, QOA), capture devices, memory buffers, AFSK modem, tone generator, noise generator, and mixers.
  - Key APIs: `audio_source_alloc_from_xfs()`, `audio_source_read()`, `audio_source_seek()`
- **Mixer**: Multi-channel audio mixing; the mixer itself can also be used as a Source.
  - Key APIs: `audio_mixer_alloc()`, `audio_mixer_add/remove()`, `audio_mixer_read()`
- **Effect**: Pluggable audio filter chain, supporting effects such as crystalizer, duplicate, IIR, mono, panning, resample, reshape, tremolo, and volume.
  - Key APIs: `audio_filter_alloc(json, len)`, `audio_effect_process()`
- **Sink**: Audio output abstraction, supporting playback devices, level meters, spectrum analysis, VAD voice activity detection, and AFSK decoding.
  - Key APIs: `audio_sink_alloc_from_playback()`, `audio_sink_write()`, `audio_sink_ioctl()`

### Command System (Command)

Unified command interface, supporting 32+ built-in commands:

```c
struct command_t {
    char * name;
    char * desc;
    void (*usage)(void);
    void (*exec)(int argc, char **argv);
};
```

Built-in commands include: aplay, autoshell, cat, cd, clear, clk, date, dcp, delay, echo, event, go, help, iplay, ls, md, mkdir, mw, net, ntpdate, pwd, reboot, rm, setting, shutdown, standby, sync, test, tscal, uniqueid, version, write.

### Core Utilities (Core)

| Module | Description | Key APIs |
|--------|-------------|----------|
| **Coroutine** | Cooperative multitasking | `coroutine_start()`, `coroutine_yield()`, `coroutine_msleep()` |
| **Logger** | Ring-buffer logging | `LOG(fmt, ...)`, `logger_enable/disable()` |
| **Profiler** | Performance profiling | `profiler_begin/end()`, `profiler_search()`, `profiler_foreach()` |
| **Setting** | Persistent key-value store | `setting_set/get/clear/sync/foreach()` |
| **ThChannel** | Thread-safe ring-buffer channel | `thchannel_alloc()`, `thchannel_send/recv(buf, len, timeout)` |
| **ThWorker** | Thread work queue | `thworker_alloc()`, `thworker_submit(func, data)`, `thworker_wait()` |
| **CoChannel** | Coroutine-safe channel (lock-free) | `cochannel_alloc()`, `cochannel_send/recv(sched, buf, len)` |
| **PSub** | Publish-subscribe system | `psub_publish(topic, data)`, `psub_subscribe(topic, cb, oneshot)` |

### Font System (Font)

Font management supporting 4 font styles (regular, italic, bold, bolditalic):

- **Font**: Font family management, install fonts from XFS or a buffer.
  - Key APIs: `font_install_from_xfs/buf()`, `font_text_bound/render()`, `font_icon_bound/render()`
- **TrueType**: TrueType/CFF font parser.
  - Key APIs: `truetype_init()`, `truetype_scale_for_pixel_height()`, `truetype_make_glyph_bitmap()`

### Shell System (Shell)

Interactive shell, supporting:

- Command auto-completion
- History navigation
- Ctrl+C interrupt handling
- Working directory management

Key APIs: `shell_system(cmdline)`, `shell_readline(prompt)`, `shell_password(prompt)`, `shell_getcwd/setcwd()`

### Time System (Time)

High-precision time management:

| Module | Description | Key APIs |
|--------|-------------|----------|
| **Timer** | High-precision timer based on a red-black tree | `timer_init()`, `timer_start()`, `timer_forward()`, `timer_cancel()` |
| **WallClock** | Wall clock time | `wallclock_gettimeofday/settimeofday()`, `wallclock_gettime/settime()` |
| **Delay** | Busy-wait delay | `ndelay(ns)`, `udelay(us)`, `mdelay(ms)` |
| **DelayCall** | One-shot delayed call | `delaycall(ms, func, data)` |

### Vision System (Vision)

Image processing algorithm library, supporting two pixel formats: `VISION_TYPE_GRAY` (8-bit grayscale) and `VISION_TYPE_RGB` (24-bit RGB888).

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

Key APIs: `vision_alloc()`, `vision_free()`, `vision_clone()`, `vision_convert()`, `vision_clear()`

Supported operations include: bitwise (bit operations), colormap (color mapping), dilate (dilation), erode (erosion), threshold (thresholding), gamma (gamma correction), gray (grayscale), sepia (sepia tone), inrange (range filtering), invert (inversion), resize (scaling), dither (dithering), rectangle (rectangle drawing), text (text rendering).

The vision system and graphics system can be converted to each other: `vision_apply_surface()` / `surface_apply_vision()`.

### Window System (Window)

Window management and event handling:

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

- Window creation & management: `window_alloc()`, `window_free()`, `window_exit()`
- Screen orientation: supports 4 rotations (0/90/180/270) and 4 flips (horizontal/vertical/main-diagonal/anti-diagonal)
- Dirty-rectangle management: `window_dirtylist_fullscreen()`, `window_dirtylist_add()`, `window_dirtylist_clear()`
- Render submission: `window_present_clear()`, `window_present_commit()`
- Event pump: `window_pump_event()`, `push_event()`
- Backlight control: `window_set_backlight()`, `window_get_backlight()`
- dp/px conversion: `window_dp_to_px()`

### File System (XFS)

Virtual file system supporting multiple mount points:

```c
struct xfs_context_t {
    struct xfs_path_t mounts;
    struct mutex_t lock;
};
```

Key APIs:

- Mount management: `xfs_mount()`, `xfs_umount()`
- File operations: `xfs_open_read/write/append()`, `xfs_read/write/seek/tell/length()`, `xfs_flush()`, `xfs_close()`
- Directory operations: `xfs_walk()`, `xfs_isdir/isfile()`, `xfs_mkdir()`, `xfs_remove()`
- Archiver: `xfs_archiver_t` provides pluggable archive format support

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

A complete 2D graphics rendering system.

### Surface Abstraction

Surface is the core object of the graphics system, with a pixel format of 32-bit premultiplied ARGB:

```c
struct surface_t {
    uint32_t *pixels;
    int width;
    int height;
    int stride;
    int format;
};
```

### Creation and Loading

- `surface_alloc(w, h)`: Create a blank Surface
- `surface_alloc_from_xfs()`: Load an image from XFS (supports QOI, PNG, JPEG)
- `surface_alloc_from_buf()`: Load from a memory buffer
- `surface_alloc_qrcode()`: Generate a QR code Surface

### Rendering Operations

- **Basic drawing**: `surface_fill()` (fill), `surface_blit()` (bit-block transfer), `surface_text()` (text), `surface_icon()` (icon)
- **Vector shapes**: `surface_shape_rectangle()`, `surface_shape_circle()`, `surface_shape_arc()`, `surface_shape_curve_to()`, etc.
- **Transforms**: `surface_shape_translate/scale/rotate/transform()`
- **Effects**: `surface_effect_glass()` (frosted glass), `surface_effect_shadow()` (shadow), `surface_effect_gradient()` (gradient), `surface_effect_checkerboard()` (checkerboard)
- **Filters**: `surface_filter_gray/sepia/invert/gamma/hue/saturate/brightness/contrast/opacity/blur()`

### Matrix Transform

2D affine transformation matrix:

```c
struct matrix2d_t {
    double a, b, c, d, e, f;
};
```

Key APIs: `matrix2d_init_identity/translate/scale/rotate()`, `matrix2d_multiply/invert()`, `matrix2d_transform_point/distance/bounds/region()`

### Dirty-Rectangle Optimization

```c
struct dirtylist_t {
    struct list_head_t list;
};
```

Key APIs: `dirtylist_alloc()`, `dirtylist_add()`, `dirtylist_clear()`, `dirtylist_merge()`, `dirtylist_clone()`

## KOBJ Virtual File System

A virtual file system similar to Linux sysfs, used to access device state and configuration.

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
- `/class/memory/meminfo`: Read memory information
- `/device/clk/clk.0/rate`: Read the clock frequency

## Publish-Subscribe System

A topic-based publish-subscribe pattern, used for event-driven architecture.

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

## Initcall Mechanism

A leveled initialization mechanism implemented via linker sections.

### Initcall Levels

```c
pure_initcall()      // 0 - Pure initialization (hash tables, device/driver lists)
machine_initcall()   // 1 - Machine initialization
core_initcall()      // 2 - Core initialization
postcore_initcall()  // 3 - Post-core initialization
driver_initcall()    // 4 - Driver initialization (most commonly used)
subsys_initcall()    // 5 - Subsystem initialization
command_initcall()   // 6 - Command initialization
server_initcall()    // 7 - Server initialization
wboxtest_initcall()  // 8 - Test initialization
late_initcall()      // 9 - Late initialization
```

### Implementation Principle

```c
/* Linker section definition */
#define __define_initcall(level, fn) \
    static initcall_t __initcall_##fn \
    __attribute__((used)) \
    __attribute__((__section__("xstar_initcall_" #level))) = fn

/* Initialization execution - in level order 0-9 */
void do_initcalls(void)
{
    initcall_t *fn;
    for(level = 0; level < 10; level++)
        for(fn = __initcall_start[level]; fn < __initcall_end[level]; fn++)
            (*fn)();
}

/* Exit execution - in reverse level order 9-0 */
void do_exitcalls(void)
{
    /* Execute exitcalls in reverse order */
}
```

## Device Tree (JSON)

Devices are configured using JSON format, which is more readable and writable than traditional DTS.

### Naming Rule

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
    },
    "uart-linux:0": {
        "device": "/dev/ttyUSB0",
        "baud-rates": 115200,
        "status": "disabled"
    }
}
```

### Property Reading

Device tree properties are read via `dt_read_*` functions:

| Function | Return Type | Description |
|----------|-------------|-------------|
| `dt_read_string(n, name, def)` | `char *` | Read a string |
| `dt_read_int(n, name, def)` | `int` | Read an integer |
| `dt_read_long(n, name, def)` | `long long` | Read a long integer |
| `dt_read_bool(n, name, def)` | `int` | Read a boolean |
| `dt_read_double(n, name, def)` | `double` | Read a double |
| `dt_read_object(n, name)` | `struct dtnode_t` | Read a sub-object |

### State Control

Setting `"status": "disabled"` skips device probing.

### Device References

Reference other devices via the `"driver-name:id"` format:

```json
{
    "led-gpio:0": {
        "gpio": "gpio-v1-linux:10",
        "active-low": true
    }
}
```

## Utility Library (LibX)

LibX provides common utility functions for algorithms, data structures, cryptography, and encoding. It is the foundation library above XOS.

### Data Structures

| Module | Description | Key APIs |
|--------|-------------|----------|
| Doubly linked list | `list.h` (header-only) | `init_list_head()`, `list_add/del/splice()`, `list_for_each_entry()` |
| Hash linked list | `list.h` (header-only) | `struct hlist_head_t`, `struct hlist_node_t` |
| Singly linked list | `slist.c/h` | Single-pointer linked list |
| FIFO ring buffer | `fifo.c/h` | Lock-free ring buffer |
| Queue | `queue.c/h` | General-purpose queue |
| Hash table | `hmap.c/h` | Key-value hash table |
| Red-black tree | `rbtree.c/h` | Self-balancing binary search tree |
| LRU cache | `lru.c/h` | Least recently used cache |
| Dynamic string | `ds.c/h` | `struct ds_t`, supports append/insert/delete/find/replace |
| KOBJ | `kobj.c/h` | Hierarchical virtual file system node |
| Device tree | `dtree.c/h` | JSON device tree parser |
| Initcall | `initcall.c/h` | Linker-section init/exit registration (10 levels) |

### Crypto/Security

| Module | File |
|--------|------|
| AES-128 | `aes128.c/h` |
| AES-256 | `aes256.c/h` |
| RC4 | `rc4.c/h` |
| ECDSA-256 | `ecdsa256.c/h` |
| SHA-1 | `sha1.c/h` |
| SHA-256 | `sha256.c/h` |
| VM crypto | `vmcrypt.c/h` |
| Reed-Solomon | `rs.c/h` (forward error correction coding) |

### Signal Processing/Algorithms

| Module | Description |
|--------|-------------|
| FFT | `fft.c/h` (Fast Fourier Transform) |
| Biquad filter | `biquad.c/h` (second-order IIR filter) |
| Kalman filter | `kalman.c/h` |
| EWMA | `ewma.c/h` (exponentially weighted moving average) |
| Median filter | `median.c/h` |
| Mean filter | `mean.c/h` |
| Time-series filter | `tsfilter.c/h` |
| Key filter | `keyfilter.c/h` (input debouncing) |
| Cosine lookup table | `costab.c/h` |
| Complex arithmetic | `complex.c/h` |
| Spring animation | `spring.c/h` |
| Backoff algorithm | `backoff.c/h` (exponential backoff) |
| Breathing LED | `breathing.c/h` |
| Easing functions | `easing.c/h` |
| Window functions | `winfunc.c/h` (Hamming, Hanning, etc.) |

### Encoding/Compression

| Module | Description |
|--------|-------------|
| Base64 | `base64.c/h` |
| JSON | `json.c/h` |
| URI | `uri.c/h` |
| QR code generation | `qrcgen.c/h` |
| Interleaver | `interleaver.c/h` |
| Hex dump | `hexdump.c/h` |
| CRC-8/16/32 | `crc8.c/h`, `crc16.c/h`, `crc32.c/h` |
| String hash | `shash.h` (header-only) |
| Character set | `charset.c/h` |
| YUV conversion | `yuv.c/h` |

### String/File Utilities

| Module | Description |
|--------|-------------|
| Path operations | `path.c/h` |
| UUID generation | `uuid.c/h` |
| Database | `db.c/h` |
| Process list | `ps.c/h` |
| Memory pool | `mm.c/h` |
| Integer square root | `sqrti.c/h` |
| Unaligned access | `unaligned.h` (header-only) |
| Byte-order conversion | `byteorder.h` (header-only, be16/le16/be32/le32) |
| BCD encoding | `bcd.h` (header-only) |
| Sorted list | `lsort.c/h` |
| Decibel conversion | `db.c/h` |
| Kernel time | `ktime.h` (header-only) |
| Core definitions | `xdef.h` (header-only, `NULL`/`TRUE`/`FALSE`/`container_of`/`XMIN`/`XMAX`) |

## Build System

### Kconfig Configuration

Uses the Kconfig system for configuration management:

```bash
make <project>/xstar.defconfig    # Apply the project default configuration
make menuconfig                    # Interactive configuration menu
```

Configuration is saved in the `.config` file, which generates `.config.h` for use by source code.

### Kbuild Compilation

Uses a Kbuild-style build system:

```makefile
obj-y += core.o                              # Always compiled
obj-$(CONFIG_DRV_CLK_FIXED) += clk-fixed.o    # Conditionally compiled
subdirs-y += adc                              # Recurse into subdirectory
```

### Project Structure

Each project lives under the `projects/` directory and contains:

- `xstar.defconfig`: Project default configuration
- `xstarcfg.h`: Project-specific type definitions and platform header
- `main.c`: Platform-specific entry point
- `linux/` or `baremetal/`: Platform implementation code
- `romdisk/`: Read-only file system (containing `boot/boot.json` device tree)

### Build Output

Build output is located in `projects/<project>/output/`, finally generating the `xstar` executable.

## Data Flow

### Device Probing Flow

```
System boot → Parse device tree (JSON) → Match driver → Call probe() → Create device instance → Register into system
```

### Coroutine Scheduling Flow

```
Coroutine start → Execute user function → Call yield() → Save context → Switch to scheduler → Select next coroutine → Restore context
```

### Graphics Rendering Flow

```
Application call → Graphics operation → Record dirty rectangle → Render to surface → G2D/software rendering → Update to framebuffer
```

### Publish-Subscribe Flow

```
Publish event → Find subscribers → Iterate callback list → Execute callback function → (Optional) Cancel oneshot subscription
```

## Extensibility

### Adding a New Driver

1. Implement the `driver_t` structure
2. Implement the `probe/remove/suspend/resume` callbacks
3. Register the driver using `driver_initcall`
4. Add the device configuration in the device tree

### Adding a New Command

1. Implement the `command_t` structure
2. Implement the `exec` callback function
3. Register the command using `command_initcall`
4. Add help documentation

### Adding a New Kernel Subsystem

1. Define the subsystem interface
2. Implement the core functionality
3. Provide the public API
4. Register using an appropriate initcall level
