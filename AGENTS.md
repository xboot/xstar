# XSTAR - Agentic Coding Guide

## Build Commands

### Build (from root directory)
```bash
make <project-name>/xstar.defconfig  # Apply config (e.g., make x64-linux-sdl-helloworld/xstar.defconfig)
make                                 # Build the project
make clean                           # Clean build artifacts (keep config)
make distclean                       # Clean everything including config
make menuconfig                      # Interactive configuration menu
```

### Build (from project directory)
```bash
cd projects/x64-linux-sdl-helloworld
make defconfig                       # Apply project-specific defconfig
make                                 # Build the project
make clean/distclean/menuconfig      # Same as root directory
```

## Code Style Guidelines

### File Header
Every source file must include MIT license header (see existing files for exact format).

### Formatting
- **Indentation**: Tabs (no spaces)
- **Line width**: Not strictly enforced, but keep it reasonable (~120 chars)
- **Brace style**: Allman (opening brace on next line) for functions and control structures; K&R (same line) for struct/enum definitions and initializer lists
- **Minimize comments**: Do not add comments to code unless explicitly requested; existing codebase has device-tree docs and struct field comments for reference

### Naming Conventions
- **Functions**: `snake_case` (e.g., `clk_fixed_probe`, `register_driver`, `xos_mem_malloc`)
- **Variables**: `snake_case` (e.g., `pdat`, `rate`, `kobj`)
- **Types**: `snake_case_t` suffix for structs/enums (e.g., `struct driver_t`, `enum device_type_t`)
- **Constants**: `ALL_CAPS_WITH_UNDERSCORES` (e.g., `DEVICE_TYPE_MAX_COUNT`, `TRUE`, `FALSE`)
- **Macros**: `ALL_CAPS` (e.g., `ARRAY_SIZE`, `container_of`, `offsetof`)
- **File names**: `kebab-case.c` (e.g., `clk-fixed.c`, `i2c-gpio.c`)

### Include Guards
Format: `__PATH_TO_FILE_H__` (e.g., `__XSTAR_DRIVER_CLK_H__`, `__XSTAR_KERNEL_GRAPHIC_SURFACE_H__`, `__XSTAR_XOS_H__`).

### Import Order
1. Local project headers (with `#include <xstar.h>` or `#include <xos/xos.h>`)
2. Driver/kernel headers (e.g., `#include <driver/clk/clk.h>`)
3. Libx headers (e.g., `#include <libx/json.h>`)
4. Standard/external headers (if any)

### Error Handling
- Return `TRUE` on success, `FALSE` on failure for boolean functions
- Return `NULL` on failure for pointer-returning functions
- Always check return values: `if(!ptr) return NULL;`
- Clean up allocations on error paths before returning

### Driver Implementation Pattern
Drivers must implement the standard lifecycle pattern:
```c
static struct device_t * <driver>_probe(struct driver_t * drv, struct dtnode_t * n);
static void <driver>_remove(struct device_t * dev);
static void <driver>_suspend(struct device_t * dev);
static void <driver>_resume(struct device_t * dev);

static struct driver_t <driver> = {
    .name     = "<driver-name>",
    .probe    = <driver>_probe,
    .remove   = <driver>_remove,
    .suspend  = <driver>_suspend,
    .resume   = <driver>_resume,
};

static void <driver>_driver_init(void) { register_driver(&<driver>); }
static void <driver>_driver_exit(void) { unregister_driver(&<driver>); }

driver_initcall(<driver>_driver_init);
driver_exitcall(<driver>_driver_exit);
```

### Initcall Levels
Use appropriate initcall macros: `pure_initcall()`, `machine_initcall()`, `core_initcall()`, `postcore_initcall()`, `driver_initcall()` (most common), `subsys_initcall()`, `command_initcall()`, `server_initcall()`, `wboxtest_initcall()`, `late_initcall()`, `final_initcall()`

### Device Tree (JSON)
Devices are configured via JSON in `/romdisk/dtree/default.json` (per-project, under `projects/<name>/romdisk/dtree/`). Use `dt_read_string()`, `dt_read_int()`, `dt_read_long()`, `dt_read_bool()` to parse properties.

### XOS Abstraction Layer
Always use XOS APIs for portability: `xos_mem_malloc()`, `xos_mem_free()`, `xos_strdup()`, `xos_strcmp()`, `xos_strtol()`, `xos_sprintf()`, `xos_mutex_lock()`, `xos_mutex_unlock()`, `xos_io_read32()`, `xos_io_write32()`. Use `NULL`, `TRUE`, `FALSE` from `libx/xdef.h`.

### Types
Use standard types available through `<xos/xos.h>` (provided via `<xstarcfg.h>`): `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, `int8_t`, `int16_t`, `int32_t`, `int64_t`, `size_t`, `ssize_t`, `io_addr_t`.

### Data Structures
Common patterns from `libx/`: Linked lists (`struct list_head_t`, `init_list_head()`, `list_add()`, `list_del()`), Hash tables (`struct hlist_head_t`, `struct hlist_node_t`), Kobj nodes for sysfs-like virtual filesystem.

### Kbuild Files
Build system uses Kbuild files to manage compilation:
```makefile
obj-y += core.o                    # Always compiled
obj-$(CONFIG_DRV_CLK_FIXED) += clk-fixed.o  # Conditional compilation
subdirs-y += subdirectory          # Include subdirectories
```

### Compiler Flags
C files compiled with: `-g -ggdb -Wall $(CONFIG_OPTIMIZE_LEVEL) -std=gnu99`
C++ files compiled with: `-g -ggdb -Wall $(CONFIG_OPTIMIZE_LEVEL)`
The optimization level defaults to `-O3` and is configurable via `CONFIG_OPTIMIZE_LEVEL` (Kconfig). No linting/formatting tools configured in build system.

### Project Structure
- `xstar/driver/` - Device drivers (51 device types)
- `xstar/kernel/` - Core subsystems (audio, command, core, font, graphic, shell, time, vision, window, xfs)
- `xstar/libx/` - Utility library (algorithms, data structures, crypto, etc.)
- `xstar/xos/` - Platform abstraction layer
- `packages/` - External packages (wboxtest, lvgl, libc, libm, libcjson, libxnes, plmpeg, vww, xaf, xui, etc.)
- `projects/` - Platform-specific configurations and code
- `tools/` - Build tools, split by host OS (`linux/kconfig`, `windows/kconfig`)
- `xstar/external/` - External libraries (jpeg, libpng, zlib, libcg, etc.)
- `developments/` - Development tools (ecdsa256-keygen, eclipse, font, mksunxi, mkz, shash, xaf)
- `documents/` - Documentation (Docusaurus site)
