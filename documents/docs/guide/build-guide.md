# XSTAR 构建指南

本文档详细说明如何构建 XSTAR 项目，包括配置选项、编译流程和常见问题。

## 目录

- [构建前准备](#构建前准备)
- [基本构建](#基本构建)
- [配置选项](#配置选项)
- [构建命令详解](#构建命令详解)
- [项目配置](#项目配置)
- [构建输出](#构建输出)
- [常见问题](#常见问题)

## 构建前准备

### 系统要求

- **操作系统**：Linux（推荐 Ubuntu 20.04 或更高版本）
- **构建工具**：Make 3.81 或更高版本
- **其他工具**：git

### 工具链安装

#### x64 平台

```bash
sudo apt-get install build-essential gcc make

# 可选：安装 SDL2 库（用于图形应用）
sudo apt-get install libsdl2-dev
```

#### ARM 平台

```bash
# ARM32 工具链
sudo apt-get install gcc-arm-linux-gnueabihf

# ARM64 工具链
sudo apt-get install gcc-aarch64-linux-gnu
```

#### RISC-V 平台

```bash
# RISC-V 工具链
sudo apt-get install gcc-riscv64-unknown-elf
```

### 交叉编译工具链

交叉编译工具链前缀通过 `CONFIG_CROSS_COMPILE` 配置项设置，各项目 defconfig 中已预设：

| 项目 | CROSS_COMPILE |
|------|---------------|
| x64-linux-sdl-helloworld | `""`（本地 GCC） |
| arm32-baremetal-t113s3 | `"arm-linux-gnueabihf-"` |
| arm64-linux-helloworld | `"aarch64-linux-gnu-"` |
| riscv32-linux-v821-helloworld | `"riscv32-linux-musl-"` |

## 基本构建

### 从根目录构建

```bash
# 1. 选择并应用项目配置
make <project-name>/xstar.defconfig

# 例如：
# make x64-linux-sdl-helloworld/xstar.defconfig
# make arm64-linux-helloworld/xstar.defconfig
# make riscv32-baremetal-v821/xstar.defconfig

# 2. 构建项目
make

# 3. 运行
./projects/<project-name>/output/xstar
```

### 从项目目录构建

```bash
# 1. 进入项目目录
cd projects/<project-name>

# 2. 应用项目配置
make defconfig

# 3. 构建项目
make

# 4. 运行
./output/xstar
```

项目 Makefile 是轻量级包装器，内部委托给根目录 Makefile 执行。

## 配置选项

### 使用 menuconfig 配置

XSTAR 提供交互式配置界面，类似 Linux 内核的 menuconfig：

```bash
# 从根目录
make menuconfig

# 从项目目录
cd projects/<project-name>
make menuconfig
```

### Kconfig 菜单结构

menuconfig 的顶层菜单如下：

1. **Compile Options** -- 编译选项
   - `CONFIG_CROSS_COMPILE`：交叉编译工具链前缀
   - Architecture type：CPU 架构选择（ARM32/ARM64/RISC-V32/RISC-V64/X32/X64）
   - `CONFIG_OPTIMIZE_LEVEL`：编译优化级别（如 `-O3`）

2. **Project Selection** -- 项目选择
   - `CONFIG_PROJECT_NAME`：项目名称
   - 各项目特定的选项

3. **Package Libraries** -- 外部包库
   - `CONFIG_PKG_LIBC`、`CONFIG_PKG_LIBM`、`CONFIG_PKG_LIBCJSON`
   - `CONFIG_PKG_LVGL`、`CONFIG_PKG_WBOXTEST`、`CONFIG_PKG_XUI` 等

4. **Xstar Options** -- XSTAR 核心选项
   - `CONFIG_XSTAR`：启用 XSTAR
   - `CONFIG_XSTAR_LOG`：启用日志
   - `CONFIG_XSTAR_LOGGER_SIZE`：日志缓冲区大小
   - `CONFIG_XSTAR_MAX_PATH`：最大路径长度
   - `CONFIG_XSTAR_WINDOW_ORIENTATION`：窗口旋转方向

5. **Device Drivers** -- 设备驱动（51 个子类别）
   - 每个设备类型下有具体驱动选项，如 `CONFIG_DRV_CLK_FIXED`、`CONFIG_DRV_I2C_GPIO`

6. **Command Configuration** -- 命令配置
   - 每个 Shell 命令可单独启用/禁用，如 `CONFIG_CMD_HELP`、`CONFIG_CMD_ECHO`

### 注意事项

- 内核子系统（audio、core、font、graphic、shell、time、vision、window、xfs）和 LibX 工具库目前无条件编译，不支持通过 Kconfig 禁用
- XOS 平台抽象层同样无条件编译

### 配置文件示例

配置保存在项目根目录的 `.config` 文件中，示例：

```makefile
CONFIG_CROSS_COMPILE=""
CONFIG_ARCH_X64=y
CONFIG_ARCH="x64"
CONFIG_OPTIMIZE_LEVEL="-O3"
CONFIG_PROJECT_NAME="x64-linux-sdl-helloworld"

CONFIG_XSTAR=y
CONFIG_XSTAR_LOG=y
CONFIG_XSTAR_LOGGER_SIZE=65536

# 驱动配置
CONFIG_DRV_CLK_FIXED=y
CONFIG_DRV_I2C_GPIO=y

# 命令配置
CONFIG_CMD_HELP=y
CONFIG_CMD_ECHO=y

# 包配置
CONFIG_PKG_LVGL=y
CONFIG_PKG_WBOXTEST=y
```

## 构建命令详解

### make

构建项目，生成可执行文件。

```bash
make -j$(nproc)
```

### make clean

清理构建产物，但保留配置文件。

```bash
make clean
```

此命令会删除：
- 编译生成的 `.o` 文件和 `.a` 静态库
- 最终的可执行文件
- 依赖文件 `.d`

但会保留：
- `.config` 配置文件
- `.config.h` 生成的头文件

### make distclean

清理所有内容，包括配置文件。

```bash
make distclean
```

此命令会删除 `clean` 删除的所有内容，还会删除：
- `.config` 配置文件
- `.config.h` 生成的头文件
- `.config.old` 旧配置文件

使用后需要重新运行 `make <project>/xstar.defconfig` 才能再次构建。

### make menuconfig

打开交互式配置界面。

### make defconfig

应用项目默认配置（仅从项目目录运行时有效）。

```bash
cd projects/<project-name>
make defconfig
```

## 项目配置

### 可用项目

| 架构 | 项目 |
|------|------|
| ARM32 baremetal | `arm32-baremetal-rk3506`, `arm32-baremetal-rv1103`, `arm32-baremetal-rv1106`, `arm32-baremetal-t113s3` |
| ARM32 Linux | `arm32-linux-helloworld` |
| ARM64 baremetal | `arm64-baremetal-t527` |
| ARM64 Linux | `arm64-linux-helloworld` |
| RISC-V32 baremetal | `riscv32-baremetal-v821`, `riscv32-baremetal-v821-test` |
| RISC-V32 FreeRTOS | `riscv32-freertos-v821` |
| RISC-V32 Linux | `riscv32-linux-v821-helloworld` 及多个 xiaozhi 变体 |
| RISC-V64 Linux | `riscv64-linux-helloworld` |
| x64 Linux | `x64-linux-helloworld`, `x64-linux-sdl-helloworld` |

### 项目目录结构

每个项目位于 `projects/<project-name>/` 下，包含：

```
projects/<project-name>/
├── Makefile              # 项目 Makefile（委托给根目录 Makefile）
├── xstar.defconfig       # 项目默认 Kconfig 配置
├── xstarcfg.h            # 项目特定类型定义和平台头文件
├── xstar.mk              # 项目特定编译/链接选项（可选）
├── main.c                # 平台特定入口点
├── linux/                # Linux 平台实现（Linux 项目）
│   └── linux.c           # XOS 环境初始化、main() 入口
├── baremetal/            # 裸机平台实现（baremetal 项目）
│   └── baremetal.c       # XOS 环境初始化、启动代码
└── romdisk/              # 只读文件系统
    ├── dtree/
    │   └── default.json  # 设备树 JSON 配置
    └── assets/           # 资源文件（字体、图片等）
```

### 添加新项目

1. 在 `projects/` 目录下创建新项目目录：

```bash
mkdir projects/my-new-project
cd projects/my-new-project
```

2. 创建 `xstar.defconfig`：

```makefile
CONFIG_CROSS_COMPILE="arm-linux-gnueabihf-"
CONFIG_ARCH_ARM32=y
CONFIG_ARCH="arm32"
CONFIG_OPTIMIZE_LEVEL="-O2"
CONFIG_PROJECT_NAME="my-new-project"
CONFIG_XSTAR=y
CONFIG_XSTAR_LOG=y
# ... 根据需要添加驱动和命令配置
```

3. 创建 `xstarcfg.h`（参考已有项目的模板）：

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

4. 创建 `Makefile`（复制已有项目的模板）：

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

5. 创建平台实现代码（`linux/linux.c` 或 `baremetal/baremetal.c`）和 `main.c`

6. 创建 `romdisk/dtree/default.json` 设备树配置

7. 构建项目：

```bash
make my-new-project/xstar.defconfig
make
```

## 构建输出

### 输出目录结构

构建成功后，输出文件位于 `projects/<project-name>/output/`：

```
projects/<project-name>/output/
└── xstar                    # 主可执行文件
```

### 可执行文件

```bash
# 从根目录运行
./projects/<project-name>/output/xstar

# 从项目目录运行
./output/xstar
```

### ROMDISK

ROMDISK 是编译时嵌入的只读文件系统，通过链接器符号 `__romdisk_start`/`__romdisk_end` 嵌入到可执行文件中。包含：

- `dtree/default.json`：设备树配置
- `assets/`：资源文件（字体、图片等）

### 调试信息

默认编译时包含调试信息（`-g -ggdb`），可以使用 GDB 调试：

```bash
gdb ./projects/<project-name>/output/xstar
```

## 常见问题

### 问题 1：找不到工具链

**错误信息**：
```
arm-linux-gnueabihf-gcc: command not found
```

**解决方案**：
安装对应平台的 GCC 工具链（参考工具链安装部分），或在 `xstar.defconfig` 中检查 `CONFIG_CROSS_COMPILE` 设置是否正确。

### 问题 2：配置文件未找到

**错误信息**：
```
make: *** No rule to make target '<project>/xstar.defconfig'. Stop.
```

**解决方案**：
检查项目名称是否正确，项目目录是否存在于 `projects/` 目录下。

### 问题 3：编译错误：头文件未找到

**错误信息**：
```
fatal error: xstarcfg.h: No such file or directory
```

**解决方案**：
确保已运行 `make <project>/xstar.defconfig` 应用配置，生成 `.config.h` 头文件。

### 问题 4：链接错误：符号未定义

**错误信息**：
```
undefined reference to 'some_function'
```

**解决方案**：
检查配置是否启用了相关驱动或模块，确保所有依赖都已启用。可通过 `make menuconfig` 检查。

### 问题 5：menuconfig 无法启动

**错误信息**：
```
make: *** No rule to make target 'menuconfig'. Stop.
```

**解决方案**：
确保已应用项目配置（运行过 `make <project>/xstar.defconfig`），并安装 ncurses 库：

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## 高级构建

### 并行构建

使用 `-j` 参数启用并行编译，加快构建速度：

```bash
make -j$(nproc)
```

### 增量编译

XSTAR 支持增量编译，只会重新编译修改过的文件。无需特殊操作，直接运行 `make` 即可。

### 交叉编译

交叉编译工具链前缀在 defconfig 中通过 `CONFIG_CROSS_COMPILE` 设置，构建系统自动使用该前缀调用 GCC、LD、AR 等工具。也可通过环境变量覆盖：

```bash
export CROSS_COMPILE=aarch64-linux-gnu-
make <project>/xstar.defconfig
make
```
