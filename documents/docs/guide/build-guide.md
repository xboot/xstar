# 构建指南

本文档说明 XSTAR 的构建配置、命令及故障排查。

## 环境与工具链

- **操作系统**：Linux（推荐 Ubuntu 24.04 或更高版本）
- **构建工具**：make、git

按目标平台安装 GCC 工具链：

```bash
# x64（本地 GCC）
sudo apt-get install build-essential gcc make

# x64 图形版（仅 x64-linux-sdl-helloworld）额外需要 SDL3
sudo apt-get install libsdl3-dev

# ARM32 / ARM64
sudo apt-get install gcc-arm-linux-gnueabihf gcc-aarch64-linux-gnu

# RISC-V
sudo apt-get install gcc-riscv64-unknown-elf
```

交叉编译前缀由各项目 `xstar.defconfig` 的 `CONFIG_CROSS_COMPILE` 预设，无需手动指定。部分示例：

| 项目 | CROSS_COMPILE |
|------|---------------|
| x64-linux-sdl-helloworld | `""`（本地 GCC） |
| arm32-baremetal-t113s3 | `arm-linux-gnueabihf-` |
| arm64-linux-helloworld | `aarch64-linux-gnu-` |
| riscv32-linux-v821-helloworld | `riscv32-linux-musl-` |

## 构建步骤

```bash
# 从根目录
make <project-name>/xstar.defconfig
make -j$(nproc)
./projects/<project-name>/output/xstar

# 或从项目目录
cd projects/<project-name>
make defconfig && make
./output/xstar
```

项目 Makefile 是轻量级包装器，委托根目录 Makefile 执行。支持增量编译，直接 `make` 只重编修改过的文件。

## make 命令

| 命令 | 作用 | 保留配置 |
|------|------|:---:|
| `make <project>/xstar.defconfig` | 应用项目默认配置 | — |
| `make` / `make -j$(nproc)` | 构建（`-j` 并行加速） | ✓ |
| `make menuconfig` | 交互式配置菜单 | ✓ |
| `make defconfig` | 项目默认配置（仅项目目录内） | ✓ |
| `make clean` | 清理 `.o`/`.a`/可执行文件/`.d`，保留 `.config` 与 `.config.h` | ✓ |
| `make distclean` | 清理全部，含 `.config`/`.config.h`/`.config.old`，需重新 defconfig | ✗ |
## 配置选项

`make menuconfig` 打开类似 Linux 内核的交互式配置，顶层菜单：

1. **Compile Options** — `CONFIG_CROSS_COMPILE`、架构（ARM32/64、RISC-V32/64、X32/X64）、`CONFIG_OPTIMIZE_LEVEL`（默认 `-O3`）
2. **Project Selection** — `CONFIG_PROJECT_NAME` 及项目特定选项
3. **Package Libraries** — `CONFIG_PKG_LIBC/LIBM/LIBCJSON/LVGL/WBOXTEST/XUI` 等
4. **Xstar Options** — `CONFIG_XSTAR`、`CONFIG_XSTAR_LOG`、`CONFIG_XSTAR_LOGGER_SIZE`、`CONFIG_XSTAR_MAX_PATH`、`CONFIG_XSTAR_WINDOW_ORIENTATION`
5. **Device Drivers** — 50+设备，每类下具体驱动如 `CONFIG_DRV_CLK_FIXED`、`CONFIG_DRV_I2C_GPIO`
6. **Command Configuration** — 每个 Shell 命令可单独启停，如 `CONFIG_CMD_HELP`、`CONFIG_CMD_ECHO`

> 内核子系统（audio/core/font/graphic/shell/time/vision/window/xfs）、LibX、XOS 目前无条件编译，不可通过 Kconfig 禁用。

配置保存在项目根目录 `.config`，并生成 `.config.h` 供源码使用。示例：

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

也可用环境变量覆盖交叉前缀：

```bash
export CROSS_COMPILE=aarch64-linux-gnu-
make <project>/xstar.defconfig
make
```

## 可用项目

| 架构 | 项目 |
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

每个项目位于 `projects/<project-name>/`，含 `Makefile`、`xstar.defconfig`、`xstarcfg.h`、`main.c`、平台实现目录（`linux/` 或 `baremetal/`）及 `romdisk/`（含 `dtree/default.json` 设备树与 `assets/` 资源）。

## 构建输出

产物为 `projects/<project-name>/output/xstar`。ROMDISK（`dtree/default.json` + `assets/`）通过链接器符号 `__romdisk_start`/`__romdisk_end` 嵌入可执行文件。默认带调试信息（`-g -ggdb`），可用 GDB 调试：

```bash
gdb ./projects/<project-name>/output/xstar
```

## 常见问题

| 现象 | 解决 |
|------|------|
| `arm-linux-gnueabihf-gcc: command not found` | 安装对应平台工具链，或检查 `CONFIG_CROSS_COMPILE` |
| `No rule to make target '<project>/xstar.defconfig'` | 项目名拼写错误或不在 `projects/` 下 |
| `fatal error: xstarcfg.h: No such file or directory` | 先运行 `make <project>/xstar.defconfig` 生成 `.config.h` |
| `undefined reference to 'some_function'` | 用 `make menuconfig` 检查相关驱动/模块是否启用 |
| `No rule to make target 'menuconfig'` | 先应用项目配置；并装 ncurses：`sudo apt-get install libncurses5-dev` |
