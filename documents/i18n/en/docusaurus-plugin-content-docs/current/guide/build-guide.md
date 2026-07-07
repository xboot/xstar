# XSTAR Build Guide

This document describes in detail how to build the XSTAR project, including configuration options, the compilation workflow, and common issues.

## Table of Contents

- [Build Prerequisites](#build-prerequisites)
- [Basic Build](#basic-build)
- [Configuration Options](#configuration-options)
- [Build Commands](#build-commands)
- [Project Configuration](#project-configuration)
- [Build Output](#build-output)
- [FAQ](#faq)

## Build Prerequisites

### System Requirements

- **Operating System**: Linux (Ubuntu 20.04 or later recommended)
- **Build Tools**: Make 3.81 or later
- **Other Tools**: git

### Toolchain Installation

#### x64 Platform

```bash
sudo apt-get install build-essential gcc make

# Optional: install SDL2 libraries (for graphical applications)
sudo apt-get install libsdl2-dev
```

#### ARM Platform

```bash
# ARM32 toolchain
sudo apt-get install gcc-arm-linux-gnueabihf

# ARM64 toolchain
sudo apt-get install gcc-aarch64-linux-gnu
```

#### RISC-V Platform

```bash
# RISC-V toolchain
sudo apt-get install gcc-riscv64-unknown-elf
```

### Cross-Compilation Toolchain

The cross-compilation toolchain prefix is set via the `CONFIG_CROSS_COMPILE` option, which is predefined in each project's defconfig:

| Project | CROSS_COMPILE |
|---------|---------------|
| x64-linux-sdl-helloworld | `""` (native GCC) |
| arm32-baremetal-t113s3 | `"arm-linux-gnueabihf-"` |
| arm64-linux-helloworld | `"aarch64-linux-gnu-"` |
| riscv32-linux-v821-helloworld | `"riscv32-linux-musl-"` |

## Basic Build

### Building from the Root Directory

```bash
# 1. Select and apply a project configuration
make <project-name>/xstar.defconfig

# For example:
# make x64-linux-sdl-helloworld/xstar.defconfig
# make arm64-linux-helloworld/xstar.defconfig
# make riscv32-baremetal-v821/xstar.defconfig

# 2. Build the project
make

# 3. Run
./projects/<project-name>/output/xstar
```

### Building from a Project Directory

```bash
# 1. Enter the project directory
cd projects/<project-name>

# 2. Apply the project configuration
make defconfig

# 3. Build the project
make

# 4. Run
./output/xstar
```

The project Makefile is a lightweight wrapper that delegates internally to the root Makefile.

## Configuration Options

### Configuring with menuconfig

XSTAR provides an interactive configuration interface, similar to the Linux kernel's menuconfig:

```bash
# From the root directory
make menuconfig

# From a project directory
cd projects/<project-name>
make menuconfig
```

### Kconfig Menu Structure

The top-level menuconfig menus are:

1. **Compile Options** -- Compile options
   - `CONFIG_CROSS_COMPILE`: Cross-compilation toolchain prefix
   - Architecture type: CPU architecture selection (ARM32/ARM64/RISC-V32/RISC-V64/X32/X64)
   - `CONFIG_OPTIMIZE_LEVEL`: Optimization level (e.g. `-O3`)

2. **Project Selection** -- Project selection
   - `CONFIG_PROJECT_NAME`: Project name
   - Project-specific options

3. **Package Libraries** -- External package libraries
   - `CONFIG_PKG_LIBC`, `CONFIG_PKG_LIBM`, `CONFIG_PKG_LIBCJSON`
   - `CONFIG_PKG_LVGL`, `CONFIG_PKG_WBOXTEST`, `CONFIG_PKG_XUI`, etc.

4. **Xstar Options** -- XSTAR core options
   - `CONFIG_XSTAR`: Enable XSTAR
   - `CONFIG_XSTAR_LOG`: Enable logging
   - `CONFIG_XSTAR_LOGGER_SIZE`: Log buffer size
   - `CONFIG_XSTAR_MAX_PATH`: Maximum path length
   - `CONFIG_XSTAR_WINDOW_ORIENTATION`: Window rotation direction

5. **Device Drivers** -- Device drivers (51 subcategories)
   - Each device type contains specific driver options, e.g. `CONFIG_DRV_CLK_FIXED`, `CONFIG_DRV_I2C_GPIO`

6. **Command Configuration** -- Command configuration
   - Each shell command can be enabled/disabled individually, e.g. `CONFIG_CMD_HELP`, `CONFIG_CMD_ECHO`

### Notes

- The kernel subsystems (audio, core, font, graphic, shell, time, vision, window, xfs) and the LibX utility library are currently compiled unconditionally and cannot be disabled via Kconfig
- The XOS platform abstraction layer is also compiled unconditionally

### Example Configuration File

Configuration is saved in the `.config` file in the project root directory. Example:

```makefile
CONFIG_CROSS_COMPILE=""
CONFIG_ARCH_X64=y
CONFIG_ARCH="x64"
CONFIG_OPTIMIZE_LEVEL="-O3"
CONFIG_PROJECT_NAME="x64-linux-sdl-helloworld"

CONFIG_XSTAR=y
CONFIG_XSTAR_LOG=y
CONFIG_XSTAR_LOGGER_SIZE=65536

# Driver configuration
CONFIG_DRV_CLK_FIXED=y
CONFIG_DRV_I2C_GPIO=y

# Command configuration
CONFIG_CMD_HELP=y
CONFIG_CMD_ECHO=y

# Package configuration
CONFIG_PKG_LVGL=y
CONFIG_PKG_WBOXTEST=y
```

## Build Commands

### make

Builds the project and generates the executable.

```bash
make -j$(nproc)
```

### make clean

Cleans build artifacts but keeps the configuration file.

```bash
make clean
```

This command deletes:
- Compiled `.o` files and `.a` static libraries
- The final executable
- Dependency files `.d`

But keeps:
- The `.config` configuration file
- The generated `.config.h` header file

### make distclean

Cleans everything, including the configuration file.

```bash
make distclean
```

This command deletes everything removed by `clean`, and also deletes:
- The `.config` configuration file
- The generated `.config.h` header file
- The `.config.old` previous configuration file

After using it, you must run `make <project>/xstar.defconfig` again before building.

### make menuconfig

Opens the interactive configuration interface.

### make defconfig

Applies the project default configuration (only valid when run from a project directory).

```bash
cd projects/<project-name>
make defconfig
```

## Project Configuration

### Available Projects

| Architecture | Projects |
|--------------|----------|
| ARM32 baremetal | `arm32-baremetal-rk3506`, `arm32-baremetal-rv1103`, `arm32-baremetal-rv1106`, `arm32-baremetal-t113s3` |
| ARM32 Linux | `arm32-linux-helloworld` |
| ARM64 baremetal | `arm64-baremetal-t527` |
| ARM64 Linux | `arm64-linux-helloworld` |
| RISC-V32 baremetal | `riscv32-baremetal-v821`, `riscv32-baremetal-v821-test` |
| RISC-V32 FreeRTOS | `riscv32-freertos-v821` |
| RISC-V32 Linux | `riscv32-linux-v821-helloworld` and several xiaozhi variants |
| RISC-V64 Linux | `riscv64-linux-helloworld` |
| x64 Linux | `x64-linux-helloworld`, `x64-linux-sdl-helloworld` |

### Project Directory Structure

Each project lives under `projects/<project-name>/` and contains:

```
projects/<project-name>/
├── Makefile              # Project Makefile (delegates to the root Makefile)
├── xstar.defconfig       # Project default Kconfig configuration
├── xstarcfg.h            # Project-specific type definitions and platform header
├── xstar.mk              # Project-specific compile/link options (optional)
├── main.c                # Platform-specific entry point
├── linux/                # Linux platform implementation (Linux projects)
│   └── linux.c           # XOS environment initialization, main() entry
├── baremetal/            # Bare-metal platform implementation (baremetal projects)
│   └── baremetal.c       # XOS environment initialization, startup code
└── romdisk/              # Read-only file system
    ├── boot/
    │   └── boot.json     # Device tree JSON configuration
    └── assets/           # Resource files (fonts, images, etc.)
```

### Adding a New Project

1. Create a new project directory under `projects/`:

```bash
mkdir projects/my-new-project
cd projects/my-new-project
```

2. Create `xstar.defconfig`:

```makefile
CONFIG_CROSS_COMPILE="arm-linux-gnueabihf-"
CONFIG_ARCH_ARM32=y
CONFIG_ARCH="arm32"
CONFIG_OPTIMIZE_LEVEL="-O2"
CONFIG_PROJECT_NAME="my-new-project"
CONFIG_XSTAR=y
CONFIG_XSTAR_LOG=y
# ... add driver and command configuration as needed
```

3. Create `xstarcfg.h` (refer to existing project templates):

```c
#ifndef __XSTARCFG_H__
#define __XSTARCFG_H__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long io_addr_t;

#endif /* __XSTARCFG_H__ */
```

4. Create the `Makefile` (copy from an existing project template):

```makefile
XSTAR_DIR     := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST))))/../..)
XSTAR_DEFCONFIG := $(notdir $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))/xstar.defconfig

.PHONY: all defconfig menuconfig clean distclean

all:
	@$(MAKE) -s -C $(XSTAR_DIR) all
defconfig:
	@$(MAKE) -s -C $(XSTAR_DIR) $(XSTAR_DEFCONFIG)
menuconfig:
	@$(MAKE) -s -C $(XSTAR_DIR) menuconfig
clean:
	@$(MAKE) -s -C $(XSTAR_DIR) clean
distclean:
	@$(MAKE) -s -C $(XSTAR_DIR) distclean
```

5. Create the platform implementation code (`linux/linux.c` or `baremetal/baremetal.c`) and `main.c`

6. Create the `romdisk/boot/boot.json` device tree configuration

7. Build the project:

```bash
make my-new-project/xstar.defconfig
make
```

## Build Output

### Output Directory Structure

After a successful build, the output files are located in `projects/<project-name>/output/`:

```
projects/<project-name>/output/
└── xstar                    # Main executable
```

### Executable

```bash
# Run from the root directory
./projects/<project-name>/output/xstar

# Run from the project directory
./output/xstar
```

### ROMDISK

The ROMDISK is a read-only file system embedded at compile time, linked into the executable via the `__romdisk_start`/`__romdisk_end` linker symbols. It contains:

- `boot/boot.json`: Device tree configuration
- `assets/`: Resource files (fonts, images, etc.)

### Debug Information

Debug information is included by default (`-g -ggdb`), so you can debug with GDB:

```bash
gdb ./projects/<project-name>/output/xstar
```

## FAQ

### Issue 1: Toolchain Not Found

**Error message**:
```
arm-linux-gnueabihf-gcc: command not found
```

**Solution**:
Install the GCC toolchain for the corresponding platform (see the toolchain installation section), or check that `CONFIG_CROSS_COMPILE` is set correctly in `xstar.defconfig`.

### Issue 2: Configuration File Not Found

**Error message**:
```
make: *** No rule to make target '<project>/xstar.defconfig'. Stop.
```

**Solution**:
Check that the project name is correct and that the project directory exists under `projects/`.

### Issue 3: Compile Error: Header File Not Found

**Error message**:
```
fatal error: xstarcfg.h: No such file or directory
```

**Solution**:
Make sure you have run `make <project>/xstar.defconfig` to apply the configuration and generate the `.config.h` header file.

### Issue 4: Link Error: Undefined Symbol

**Error message**:
```
undefined reference to 'some_function'
```

**Solution**:
Check whether the relevant driver or module is enabled in the configuration, and make sure all dependencies are enabled. You can inspect this via `make menuconfig`.

### Issue 5: menuconfig Fails to Start

**Error message**:
```
make: *** No rule to make target 'menuconfig'. Stop.
```

**Solution**:
Make sure a project configuration has been applied (run `make <project>/xstar.defconfig` once) and that the ncurses library is installed:

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## Advanced Build

### Parallel Build

Use the `-j` option to enable parallel compilation and speed up the build:

```bash
make -j$(nproc)
```

### Incremental Build

XSTAR supports incremental compilation, recompiling only the files that have changed. No special action is required; just run `make` directly.

### Cross Compilation

The cross-compilation toolchain prefix is set via `CONFIG_CROSS_COMPILE` in the defconfig, and the build system automatically uses this prefix to invoke GCC, LD, AR, and other tools. It can also be overridden via an environment variable:

```bash
export CROSS_COMPILE=aarch64-linux-gnu-
make <project>/xstar.defconfig
make
```
