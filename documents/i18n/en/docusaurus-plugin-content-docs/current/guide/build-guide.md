# Build Guide

This document describes the build configuration, commands, and troubleshooting for XSTAR. For a quick start, see [Quick Start](./quick-start); to create a new project, see [Development Guide - Adding a New Project](./development-guide#adding-a-new-project).

## Environment & Toolchains

- **Operating System**: Linux (Ubuntu 24.04 or later recommended)
- **Build Tools**: make, git

Install the GCC toolchain for the target platform:

```bash
# x64 (native GCC)
sudo apt-get install build-essential gcc make

# x64 graphical edition (x64-linux-sdl-helloworld only) additionally requires SDL3
sudo apt-get install libsdl3-dev

# ARM32 / ARM64
sudo apt-get install gcc-arm-linux-gnueabihf gcc-aarch64-linux-gnu

# RISC-V
sudo apt-get install gcc-riscv64-unknown-elf
```

The cross-compile prefix is preset by each project's `xstar.defconfig` via `CONFIG_CROSS_COMPILE`; no manual specification is required. Some examples:

| Project | CROSS_COMPILE |
|------|---------------|
| x64-linux-sdl-helloworld | `""` (native GCC) |
| arm32-baremetal-t113s3 | `arm-linux-gnueabihf-` |
| arm64-linux-helloworld | `aarch64-linux-gnu-` |
| riscv32-linux-v821-helloworld | `riscv32-linux-musl-` |

## Build Steps

```bash
# From the root directory
make <project-name>/xstar.defconfig
make -j$(nproc)
./projects/<project-name>/output/xstar

# Or from the project directory
cd projects/<project-name>
make defconfig && make
./output/xstar
```

The project Makefile is a lightweight wrapper that delegates to the root Makefile. Incremental builds are supported; running `make` directly only recompiles modified files.

## make Commands

| Command | Description | Keeps Config |
|------|------|:---:|
| `make <project>/xstar.defconfig` | Apply project default configuration | — |
| `make` / `make -j$(nproc)` | Build (`-j` for parallel acceleration) | ✓ |
| `make menuconfig` | Interactive configuration menu | ✓ |
| `make defconfig` | Project default configuration (only within project directory) | ✓ |
| `make clean` | Clean `.o`/`.a`/executables/`.d`, keeps `.config` and `.config.h` | ✓ |
| `make distclean` | Clean everything, including `.config`/`.config.h`/`.config.old`; requires re-running defconfig | ✗ |
## Configuration Options

`make menuconfig` opens a Linux kernel-like interactive configuration. Top-level menus:

1. **Compile Options** — `CONFIG_CROSS_COMPILE`, architecture (ARM32/64, RISC-V32/64, X32/X64), `CONFIG_OPTIMIZE_LEVEL` (default `-O3`)
2. **Project Selection** — `CONFIG_PROJECT_NAME` and project-specific options
3. **Package Libraries** — `CONFIG_PKG_LIBC/LIBM/LIBCJSON/LVGL/WBOXTEST/XUI`, etc.
4. **Xstar Options** — `CONFIG_XSTAR`, `CONFIG_XSTAR_LOG`, `CONFIG_XSTAR_LOGGER_SIZE`, `CONFIG_XSTAR_MAX_PATH`, `CONFIG_XSTAR_WINDOW_ORIENTATION`
5. **Device Drivers** — 50+ devices, with specific drivers under each category such as `CONFIG_DRV_CLK_FIXED`, `CONFIG_DRV_I2C_GPIO`
6. **Command Configuration** — Each Shell command can be individually enabled/disabled, e.g. `CONFIG_CMD_HELP`, `CONFIG_CMD_ECHO`

> Kernel subsystems (audio/core/font/graphic/shell/time/vision/window/xfs), LibX, and XOS are currently compiled unconditionally and cannot be disabled via Kconfig.

Configuration is saved in the project root `.config`, and `.config.h` is generated for source code use. Example:

```makefile
CONFIG_CROSS_COMPILE=""
CONFIG_ARCH_X64=y
CONFIG_ARCH="x64"
CONFIG_OPTIMIZE_LEVEL="-O3"
CONFIG_PROJECT_NAME="x64-linux-sdl-helloworld"
CONFIG_XSTAR=y
CONFIG_XSTAR_LOG=y
CONFIG_DRV_CLK_FIXED=y
CONFIG_CMD_HELP=y
CONFIG_PKG_LVGL=y
```

You can also override the cross prefix using an environment variable:

```bash
export CROSS_COMPILE=aarch64-linux-gnu-
make <project>/xstar.defconfig
make
```

## Available Projects

| Architecture | Project |
|------|------|
| ARM32 baremetal | `arm32-baremetal-rk3506`, `arm32-baremetal-rv1103`, `arm32-baremetal-rv1106`, `arm32-baremetal-t113s3` |
| ARM32 Linux | `arm32-linux-helloworld` |
| ARM64 baremetal | `arm64-baremetal-t527` |
| ARM64 Linux | `arm64-linux-helloworld` |
| RISC-V32 baremetal | `riscv32-baremetal-v821`, `riscv32-baremetal-v821-test` |
| RISC-V32 FreeRTOS | `riscv32-freertos-v821` |
| RISC-V32 Linux | `riscv32-linux-v821-helloworld` |
| RISC-V64 Linux | `riscv64-linux-helloworld` |
| x64 Linux | `x64-linux-helloworld`, `x64-linux-sdl-helloworld` |

Each project is located in `projects/<project-name>/`, containing a `Makefile`, `xstar.defconfig`, `xstarcfg.h`, `main.c`, a platform implementation directory (`linux/` or `baremetal/`), and a `romdisk/` (containing the `dtree/default.json` device tree and `assets/` resources).

## Build Output

The artifact is `projects/<project-name>/output/xstar`. The ROMDISK (`dtree/default.json` + `assets/`) is embedded into the executable via the linker symbols `__romdisk_start`/`__romdisk_end`. Debug info is included by default (`-g -ggdb`), allowing debugging with GDB:

```bash
gdb ./projects/<project-name>/output/xstar
```

## Troubleshooting

| Symptom | Solution |
|------|------|
| `arm-linux-gnueabihf-gcc: command not found` | Install the toolchain for the target platform, or check `CONFIG_CROSS_COMPILE` |
| `No rule to make target '<project>/xstar.defconfig'` | Project name is misspelled or not under `projects/` |
| `fatal error: xstarcfg.h: No such file or directory` | Run `make <project>/xstar.defconfig` first to generate `.config.h` |
| `undefined reference to 'some_function'` | Use `make menuconfig` to check whether the relevant driver/module is enabled |
| `No rule to make target 'menuconfig'` | Apply the project configuration first; also install ncurses: `sudo apt-get install libncurses5-dev` |
