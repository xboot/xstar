# XSTAR Development Guide

This document provides a detailed guide for XSTAR development, including code conventions, driver development, command development, testing, and more.

## Table of Contents

- [Code Conventions](#code-conventions)
- [Driver Development](#driver-development)
- [Command Development](#command-development)
- [Kernel Subsystem Development](#kernel-subsystem-development)
- [Test Development](#test-development)
- [XOS Platform Porting](#xos-platform-porting)
- [Common Development Tasks](#common-development-tasks)
- [Debugging Tips](#debugging-tips)

## Code Conventions

### File Naming

- Source files: `kebab-case.c` (e.g. `clk-fixed.c`, `i2c-gpio.c`)
- Header files: `kebab-case.h` (same name as the source file)

### File Header Comment

Every source file must contain the MIT license header:

```c
/*
 * driver/clk/clk-fixed.c
 *
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
```

### Code Format

- **Indentation**: Use tabs (not spaces)
- **Line width**: About 120 characters (not strictly enforced, but keep it reasonable)
- **Braces**: Functions and control structures use Allman style (opening brace on the next line); struct/enum definitions and initializer lists use K&R style (opening brace on the same line)
- **Comments**: Keep comments to a minimum; do not proactively add code comments (unless explicitly required); existing device-tree documentation comments and struct field comments in the code are for reference only

### Naming Conventions

- **Functions**: `snake_case` (e.g. `clk_fixed_probe`, `register_driver`)
- **Variables**: `snake_case` (e.g. `pdat`, `rate`, `kobj`)
- **Types**: `snake_case_t` suffix (e.g. `struct driver_t`, `enum device_type_t`)
- **Constants**: `ALL_CAPS_WITH_UNDERSCORES` (e.g. `DEVICE_TYPE_MAX_COUNT`, `TRUE`, `FALSE`)
- **Macros**: `ALL_CAPS` (e.g. `ARRAY_SIZE`, `container_of`, `offsetof`)

### Header Guards

Format: `__PATH_TO_FILE_H__`. Example:

```c
#ifndef __XSTAR_DRIVER_CLK_H__
#define __XSTAR_DRIVER_CLK_H__

/* Header content */

#endif /* __XSTAR_DRIVER_CLK_H__ */
```

### Include Order

```c
#include <xstar.h>
#include <xos/xos.h>

#include <driver/clk/clk.h>

#include <libx/json.h>
```

1. Local project headers (`#include <xstar.h>` or `#include <xos/xos.h>`)
2. Driver/kernel headers (e.g. `#include <driver/clk/clk.h>`)
3. LibX headers (e.g. `#include <libx/json.h>`)
4. Standard/external headers (if any)

### Error Handling

- Boolean functions: return `TRUE` on success, `FALSE` on failure
- Pointer functions: return a pointer on success, `NULL` on failure
- Always check return values
- Free allocated resources in the error path

```c
static struct device_t * my_probe(struct driver_t * drv, struct dtnode_t * n)
{
    struct xxx_pdata_t *pdat;
    struct xxx_t *xxx;

    pdat = xos_mem_malloc(sizeof(struct xxx_pdata_t));
    if(!pdat)
        return NULL;

    xxx = xos_mem_malloc(sizeof(struct xxx_t));
    if(!xxx)
    {
        xos_mem_free(pdat);
        return NULL;
    }

    /* Initialize and register */

    return dev;
}
```

### Type Usage

Use standard types (provided via `<xos/xos.h>` through `<xstarcfg.h>`):

- `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`
- `int8_t`, `int16_t`, `int32_t`, `int64_t`
- `size_t`, `ssize_t`
- `io_addr_t`
- `NULL`, `TRUE`, `FALSE` (from `libx/xdef.h`)

## Driver Development

### Driver Template

```c
#include <xstar.h>
#include <driver/xxx/xxx.h>

struct xxx_pdata_t {
    /* Private data */
};

static struct device_t * xxx_probe(struct driver_t * drv, struct dtnode_t * n)
{
    struct xxx_pdata_t *pdat;

    /* Parse device tree properties */
    const char *name = dt_read_string(n, "name", NULL);
    int value = dt_read_int(n, "value", 0);

    /* Allocate private data */
    pdat = xos_mem_malloc(sizeof(struct xxx_pdata_t));
    if(!pdat)
        return NULL;

    /* Initialize hardware */

    /* Create and register the device */
    return register_xxx(drv, pdat);
}

static void xxx_remove(struct device_t * dev)
{
    struct xxx_pdata_t *pdat = (struct xxx_pdata_t *)dev->priv;

    /* Release resources */

    xos_mem_free(pdat);
}

static void xxx_suspend(struct device_t * dev)
{
}

static void xxx_resume(struct device_t * dev)
{
}

static struct driver_t xxx_driver = {
    .name    = "xxx-driver",
    .probe   = xxx_probe,
    .remove  = xxx_remove,
    .suspend = xxx_suspend,
    .resume  = xxx_resume,
};

static void xxx_driver_init(void)
{
    register_driver(&xxx_driver);
}

static void xxx_driver_exit(void)
{
    unregister_driver(&xxx_driver);
}

driver_initcall(xxx_driver_init);
driver_exitcall(xxx_driver_exit);
```

### Device Tree Configuration

Add the device configuration in `romdisk/dtree/default.json`:

```json
{
    "xxx-driver:0": {
        "name": "my-device",
        "value": 42
    }
}
```

The device tree key format is `"driver-name:id@address"`:
- `driver-name`: Driver name, must match `driver_t.name`
- `id`: Device instance number (optional)
- `address`: Physical address (optional)

Setting `"status": "disabled"` skips device probing.

### Device Property Reading

| Function | Return Type | Description |
|----------|-------------|-------------|
| `dt_read_string(n, name, def)` | `char *` | Read a string |
| `dt_read_int(n, name, def)` | `int` | Read an integer |
| `dt_read_long(n, name, def)` | `long long` | Read a long integer |
| `dt_read_bool(n, name, def)` | `int` | Read a boolean |
| `dt_read_double(n, name, def)` | `double` | Read a double |
| `dt_read_object(n, name)` | `struct dtnode_t` | Read a sub-object |

### Device References

Reference other devices in JSON via the `"driver-name:id"` format:

```json
{
    "led-gpio:0": {
        "gpio": "gpiochip0:10",
        "active-low": true
    }
}
```

Parse the reference in code and look up the device:

```c
static struct device_t * xxx_probe(struct driver_t * drv, struct dtnode_t * n)
{
    const char *gpio_name = dt_read_string(n, "gpio", NULL);
    struct device_t *gpio_dev = search_device(gpio_name, DEVICE_TYPE_GPIOCHIP);
    if(!gpio_dev)
        return NULL;
    /* Use gpio_dev */
}
```

### Kbuild File

Create a `Kbuild` file in the driver directory:

```makefile
obj-y += core.o
obj-$(CONFIG_DRV_XXX) += xxx-driver.o
```

### Device Types

The system defines 51 device types (`enum device_type_t`). When registering a device, the driver must specify the corresponding type. Common types:

| Category | Type Enum |
|----------|-----------|
| Clock | `DEVICE_TYPE_CLK`, `DEVICE_TYPE_CLOCKEVENT`, `DEVICE_TYPE_CLOCKSOURCE` |
| GPIO | `DEVICE_TYPE_GPIOCHIP`, `DEVICE_TYPE_IRQCHIP`, `DEVICE_TYPE_RESETCHIP` |
| Communication | `DEVICE_TYPE_I2C`, `DEVICE_TYPE_SPI`, `DEVICE_TYPE_UART`, `DEVICE_TYPE_NET` |
| Display | `DEVICE_TYPE_FRAMEBUFFER`, `DEVICE_TYPE_G2D`, `DEVICE_TYPE_CONSOLE` |
| Audio | `DEVICE_TYPE_AUDIOCAPTURE`, `DEVICE_TYPE_AUDIOPLAYBACK` |
| Input | `DEVICE_TYPE_INPUT`, `DEVICE_TYPE_CAMERA` |
| Storage | `DEVICE_TYPE_BLOCK`, `DEVICE_TYPE_NVMEM` |
| Output | `DEVICE_TYPE_LED`, `DEVICE_TYPE_PWM`, `DEVICE_TYPE_SERVO` |

See `xstar/driver/device.h` for the complete list.

## Command Development

### Command Structure

```c
struct command_t
{
    struct list_head_t list;
    const char * name;
    const char * desc;
    void (*usage)(void);
    int (*exec)(int argc, char ** argv);
};
```

### Command Template

```c
#include <xstar.h>

static void mycmd_usage(void)
{
    shell_printf("Usage: mycmd [options]\n");
    shell_printf("  mycmd        - do something\n");
    shell_printf("  mycmd -h     - show help\n");
}

static int mycmd_exec(int argc, char ** argv)
{
    shell_printf("My command executed\n");

    if(argc > 1)
    {
        shell_printf("Argument: %s\n", argv[1]);
    }

    return 0;
}

static struct command_t mycmd = {
    .name  = "mycmd",
    .desc  = "My command description",
    .usage = mycmd_usage,
    .exec  = mycmd_exec,
};

static void mycmd_init(void)
{
    register_command(&mycmd);
}

static void mycmd_exit(void)
{
    unregister_command(&mycmd);
}

command_initcall(mycmd_init);
command_exitcall(mycmd_exit);
```

### Command Usage

```bash
# Execute the command in the shell
mycmd
mycmd arg1 arg2
```

### Kbuild File

```makefile
obj-$(CONFIG_CMD_MYCMD) += cmd-mycmd.o
```

The Kconfig option prefix for commands is `CONFIG_CMD_*`; each command can be enabled/disabled individually. The core framework `command.o` is always compiled (`obj-y`).

## Kernel Subsystem Development

### Subsystem Interface Definition

```c
/* kernel/mysubsystem/my-subsystem.h */
#ifndef __XSTAR_KERNEL_MYSUBSYSTEM_MY_SUBSYSTEM_H__
#define __XSTAR_KERNEL_MYSUBSYSTEM_MY_SUBSYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

struct my_subsystem_handle_t;

struct my_subsystem_handle_t * my_subsystem_open(void);
void my_subsystem_close(struct my_subsystem_handle_t *handle);
int my_subsystem_do_something(struct my_subsystem_handle_t *handle, int arg);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_MYSUBSYSTEM_MY_SUBSYSTEM_H__ */
```

### Subsystem Implementation

```c
/* kernel/mysubsystem/my-subsystem.c */
#include <xstar.h>
#include <kernel/mysubsystem/my-subsystem.h>

struct my_subsystem_handle_t {
    /* Private data */
};

struct my_subsystem_handle_t * my_subsystem_open(void)
{
    struct my_subsystem_handle_t *handle;

    handle = xos_mem_malloc(sizeof(struct my_subsystem_handle_t));
    if(!handle)
        return NULL;

    /* Initialize */

    return handle;
}

void my_subsystem_close(struct my_subsystem_handle_t *handle)
{
    if(handle)
    {
        /* Cleanup */
        xos_mem_free(handle);
    }
}

int my_subsystem_do_something(struct my_subsystem_handle_t *handle, int arg)
{
    if(!handle)
        return -1;

    /* Implement functionality */

    return 0;
}
```

### Subsystem Initialization

```c
static void my_subsystem_init(void)
{
    /* Initialize the subsystem */
}

subsys_initcall(my_subsystem_init);
```

### Kbuild File

Kernel subsystems are currently compiled unconditionally; the Kbuild file uses `obj-y`:

```makefile
obj-y += my-subsystem.o
```

## Test Development

### Test Framework

XSTAR uses the wboxtest testing framework, located at `packages/wboxtest-0.0.0/`.

### Test Structure

```c
struct wboxtest_t
{
    struct kobj_t * kobj;
    struct hlist_node_t node;
    const char * group;
    const char * name;
    void * (*setup)(struct wboxtest_t * wbt);
    void (*clean)(struct wboxtest_t * wbt, void * data);
    void (*run)(struct wboxtest_t * wbt, void * data);
};
```

### Test Template

```c
#include <wboxtest.h>

struct wbt_xxx_pdata_t {
    /* Test private data */
};

static void * xxx_setup(struct wboxtest_t * wbt)
{
    struct wbt_xxx_pdata_t *pdat;

    pdat = xos_mem_malloc(sizeof(struct wbt_xxx_pdata_t));
    if(!pdat)
        return NULL;

    /* Initialize the test environment */

    return pdat;
}

static void xxx_clean(struct wboxtest_t * wbt, void * data)
{
    struct wbt_xxx_pdata_t *pdat = (struct wbt_xxx_pdata_t *)data;

    if(pdat)
    {
        /* Clean up the test environment */
        xos_mem_free(pdat);
    }
}

static void xxx_run(struct wboxtest_t * wbt, void * data)
{
    struct wbt_xxx_pdata_t *pdat = (struct wbt_xxx_pdata_t *)data;

    if(!pdat)
        return;

    /* Test logic */
    assert_true(condition);
    assert_equal(expected, actual);
}

static struct wboxtest_t wbt_xxx = {
    .group = "mygroup",
    .name  = "xxx",
    .setup = xxx_setup,
    .clean = xxx_clean,
    .run   = xxx_run,
};

static void xxx_wbt_init(void)
{
    register_wboxtest(&wbt_xxx);
}

static void xxx_wbt_exit(void)
{
    unregister_wboxtest(&wbt_xxx);
}

wboxtest_initcall(xxx_wbt_init);
wboxtest_exitcall(xxx_wbt_exit);
```

### Assertion Macros

| Macro | Description |
|-------|-------------|
| `assert_null(x)` | Assert that x is NULL |
| `assert_not_null(x)` | Assert that x is not NULL |
| `assert_true(x)` | Assert that x is true |
| `assert_false(x)` | Assert that x is false |
| `assert_equal(a, b)` | Assert equality |
| `assert_not_equal(a, b)` | Assert inequality |
| `assert_string_equal(a, b)` | Assert strings are equal |
| `assert_string_not_equal(a, b)` | Assert strings are not equal |
| `assert_memory_equal(a, b, l)` | Assert memory is equal |
| `assert_memory_not_equal(a, b, l)` | Assert memory is not equal |
| `assert_inrange(v, min, max)` | Assert value is in range |
| `assert_not_inrange(v, min, max)` | Assert value is not in range |

### Running Tests

```bash
# Run all tests
wboxtest

# List all tests
wboxtest -l

# Run all tests in a group
wboxtest thread

# Run a specific test
wboxtest thread mutex

# Run a specific test N times
wboxtest thread mutex -c 100
```

### Kbuild File

```makefile
obj-$(CONFIG_WBOXTEST_MYGROUP) += xxx.o
```

The core framework is controlled by `CONFIG_PKG_WBOXTEST`, and sub-test groups are controlled by `CONFIG_WBOXTEST_*`.

## XOS Platform Porting

Porting XOS to a new platform requires implementing the `xos_environ_t` function pointer table and passing it to `xstar_init()` in the project's `main.c`.

### Porting Steps

1. Create a platform implementation file in the project directory (e.g. `linux/linux.c` or `baremetal/baremetal.c`)
2. Implement the required function pointers in `xos_environ_t`
3. Build an `xos_environ_t` instance in `main.c` and call `xstar_init(&env, NULL)`
4. For coroutine support, write architecture-specific coroutine assembly code

### xos_environ_t Interface

```c
static struct xos_environ_t env = {
    .mem = {
        .malloc    = my_malloc,
        .free      = my_free,
        .realloc   = my_realloc,
    },
    .dma = {
        .alloc_coherent     = my_dma_alloc_coherent,
        .free_coherent      = my_dma_free_coherent,
        .alloc_noncoherent  = my_dma_alloc_noncoherent,
        .free_noncoherent   = my_dma_free_noncoherent,
        .sync               = my_dma_sync,
    },
    .io = {
        .read8   = my_read8,
        .read16  = my_read16,
        .read32  = my_read32,
        .read64  = my_read64,
        .write8  = my_write8,
        .write16 = my_write16,
        .write32 = my_write32,
        .write64 = my_write64,
    },
    .stdio = {
        .read  = my_stdio_read,
        .write = my_stdio_write,
    },
    .pm = {
        .shutdown = my_shutdown,
        .reboot   = my_reboot,
        .standby  = my_standby,
    },
    .copyright = {
        .uniqueid = my_uniqueid,
        .verify   = my_verify,
    },
    .file = {
        .cwd     = my_cwd,
        .open    = my_file_open,
        .close   = my_file_close,
        .read    = my_file_read,
        .write   = my_file_write,
        .seek    = my_file_seek,
        .tell    = my_file_tell,
        .length  = my_file_length,
        .sync    = my_file_sync,
        .mkdir   = my_mkdir,
        .remove  = my_remove,
        .access  = my_access,
        .isdir   = my_isdir,
        .isfile  = my_isfile,
        .mode    = my_mode,
        .walk    = my_walk,
    },
    .coroutine = {
        .make = my_coroutine_make,
        .jump = my_coroutine_jump,
    },
    .thread = {
        .create  = my_thread_create,
        .destroy = my_thread_destroy,
        .wait    = my_thread_wait,
        .sleep   = my_thread_sleep,
    },
    .mutex = {
        .init   = my_mutex_init,
        .exit   = my_mutex_exit,
        .lock   = my_mutex_lock,
        .trylock = my_mutex_trylock,
        .unlock = my_mutex_unlock,
    },
    .semaphore = {
        .init = my_semaphore_init,
        .exit = my_semaphore_exit,
        .wait = my_semaphore_wait,
        .post = my_semaphore_post,
    },
    .other = {
        /* Other platform-specific operations */
    },
};

int main(int argc, char * argv[])
{
    platform_init();
    xstar_init(&env, NULL);
    shell_system("shell;");
    xstar_exit();
    platform_exit();
    return 0;
}
```

`xos_environ_init(env)` installs the non-NULL function pointers into the global `__xos_environ`; entries that are not set keep their default empty implementations.

### Coroutine Porting

Two coroutine primitives must be implemented for the specific architecture: `coroutine_make` and `coroutine_jump`.

```c
/* x64-coroutine.S */
.global x64_coroutine_make
x64_coroutine_make:
    /* Create the coroutine context on the stack, set the entry function */
    /* Args: RDI=stack, RSI=size, RDX=func */
    /* Return: RAX=context pointer */

.global x64_coroutine_jump
x64_coroutine_jump:
    /* Save the current context, switch to the target context */
    /* Args: RDI=target_ctx, RSI=priv */
    /* Return: struct co_transfer_t { fctx, priv } */
    stp x19, x20, [x1], #16
    /* ... save/restore callee-saved registers ... */
    ret
```

The naming convention is `<arch>_coroutine_make` and `<arch>_coroutine_jump`, and the assembly file lives in the project's platform directory.

Existing architecture implementations:
- x64: `projects/x64-linux-sdl-helloworld/linux/x64-coroutine.S`
- ARM64: `arm64-coroutine.S` in bare-metal projects
- RISC-V32/64: `riscv32-coroutine.S` / `riscv64-coroutine.S` in the corresponding projects

## Common Development Tasks

### Adding a New Device Type

1. Add the new type to `enum device_type_t` in `xstar/driver/device.h`
2. Update the `__device_head` array size and the type name table in `xstar/driver/device.c`
3. Create the corresponding device interface header (e.g. `xstar/driver/xxx/xxx.h`)
4. Implement device registration/search functions (e.g. `register_xxx`, `search_xxx`)
5. Use the new device type in drivers

### Adding a New Kernel Subsystem

1. Create a subsystem directory under `xstar/kernel/`
2. Define the subsystem interface header
3. Implement the subsystem functionality
4. Register using an appropriate initcall level
5. Add build rules in `xstar/kernel/Kbuild` (kernel subsystems are currently compiled unconditionally)

### Adding a New LibX Utility Function

1. Create `.c` and `.h` files under `xstar/libx/`
2. Implement the function
3. Add build rules in `xstar/libx/Kbuild` (LibX is currently compiled unconditionally)

### Adding a New External Package

1. Create a package directory under `packages/` (e.g. `mypkg-1.0.0/`)
2. Write the `Kbuild` file
3. Add a configuration option in `packages/Kconfig`
4. Enable the package in the project defconfig

## Debugging Tips

### Using GDB

Debug information is included by default (`-g -ggdb`), so you can debug directly with GDB:

```bash
# Start GDB
gdb ./projects/<project-name>/output/xstar

# Common GDB commands
break <function>    # Set a breakpoint
run                 # Run
next                # Step over (skip function calls)
step                # Step into (enter function calls)
continue            # Continue execution
print <var>         # Print a variable
backtrace           # View the call stack
```

### Using Shell Commands

XSTAR provides rich shell commands for debugging:

```bash
# List devices
ls /kobj/device/

# View device information
cat /kobj/device/framebuffer/fb.0/width

# View memory information
cat /kobj/class/memory/meminfo

# View clock information
clk

# View date and time
date

# Run tests
wboxtest -l
wboxtest thread mutex -c 10
```

### Using Log Output

```c
/* Shell output (for commands and interactive code) */
shell_printf("Debug message: %d\n", value);

/* Log system (requires CONFIG_XSTAR_LOG) */
LOG("value = %d\n", value);
```

### Exporting Debug Info via KOBJ

```c
static ssize_t my_debug_read(struct kobj_t * kobj, void * buf, size_t size)
{
    return xos_snprintf(buf, size, "Debug info: %d\n", my_value);
}

/* Create a KOBJ debug node in the device probe */
kobj_add_regular(dev->kobj, "debug", my_debug_read, NULL, NULL);
```

Common KOBJ API functions:

| Function | Description |
|----------|-------------|
| `kobj_alloc_directory(name)` | Allocate a directory node |
| `kobj_alloc_regular(name, read, write, priv)` | Allocate a file node |
| `kobj_add(parent, kobj)` | Add a child node |
| `kobj_remove(parent, kobj)` | Remove a child node |
| `kobj_add_directory(parent, name)` | Create and add a directory in one step |
| `kobj_add_regular(parent, name, read, write, priv)` | Create and add a file in one step |
| `kobj_search(parent, name)` | Search for a child node |
| `kobj_search_directory_with_create(parent, name)` | Search for a directory, create it if it does not exist |

## Best Practices

### Memory Management

- Always check whether memory allocation succeeded
- Free already-allocated memory in error paths
- Use the XOS API (`xos_mem_malloc`/`xos_mem_free`) instead of platform-native functions

### Error Handling

- Boolean functions return `TRUE`/`FALSE`; pointer functions return `NULL` on failure
- Clean up allocated resources layer by layer in error paths
- On device probe failure, free all allocated memory before returning `NULL`

### Portability

- Always use the XOS API (`xos_io_read32`, `xos_mem_malloc`, etc.) instead of platform-native functions
- Use `TRUE`/`FALSE`/`NULL` from `libx/xdef.h` instead of `<stdbool.h>`
- Use standard integer types (e.g. `uint32_t`) instead of platform-specific types

### Code Reuse

- Use the common functions provided by LibX (data structures, algorithms, cryptography, etc.)
- Reuse device class interfaces between drivers (e.g. `register_clk`, `search_gpiochip`)
- Extract common code into standalone functions
