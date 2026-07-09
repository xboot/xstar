# 快速开始

仅需几分钟，构建并运行第一个 XSTAR 项目。

## 环境要求

- **操作系统**：Linux（推荐 Ubuntu 24.04 或更高）
- **基础工具**：gcc、make、git
- **图形版（可选）**：SDL3（仅 `x64-linux-sdl-helloworld` 需要）

```bash
sudo apt-get install build-essential make git
```

## 构建并运行

最简项目 `x64-linux-helloworld` 无外部依赖，构建后进入交互式 Shell：

```bash
# 从仓库根目录
make x64-linux-helloworld/xstar.defconfig
make -j$(nproc)

# 运行（进入 XSTAR Shell）
./projects/x64-linux-helloworld/output/xstar
```

## 体验图形界面（可选）

带 SDL 窗口的版本需要先安装 SDL3：

```bash
sudo apt-get install libsdl3-dev
make x64-linux-sdl-helloworld/xstar.defconfig
make -j$(nproc)
./projects/x64-linux-sdl-helloworld/output/xstar
```

## 运行后试试这些命令

进入 Shell 后，可执行内置命令：

```bash
help              # 列出所有命令
version           # 查看版本信息
ls /              # 列出根目录
ls /kobj/device/  # 查看已注册设备
date              # 查看日期时间
echo hello        # 回显
```

## 交叉编译其他平台

工具链前缀已在各项目 defconfig 中预设，换项目即可。所需交叉工具链按平台安装：

```bash
# ARM64 Linux
sudo apt-get install gcc-aarch64-linux-gnu
make arm64-linux-helloworld/xstar.defconfig
make -j$(nproc)

# ARM32 baremetal（如 T113S3）
sudo apt-get install gcc-arm-linux-gnueabihf
make arm32-baremetal-t113s3/xstar.defconfig
make -j$(nproc)

# RISC-V32 Linux（如 V821）
make riscv32-linux-v821-helloworld/xstar.defconfig
make -j$(nproc)
```

## 常用 make 命令

| 命令 | 作用 |
|------|------|
| `make <project>/xstar.defconfig` | 应用项目默认配置 |
| `make` | 构建 |
| `make -j$(nproc)` | 并行构建（更快） |
| `make menuconfig` | 交互式配置菜单 |
| `make clean` | 清理构建产物，保留配置 |
| `make distclean` | 清理全部，含配置文件 |

也可以在项目目录内操作（Makefile 会委托根目录执行）：

```bash
cd projects/x64-linux-sdl-helloworld
make distclean
make defconfig
make
```

## 获取帮助

- **源码仓库**：[GitHub](https://github.com/xboot/xstar)
- **镜像仓库**：[Gitee](https://gitee.com/xboot/xstar)
- **延伸阅读**：[知乎专栏](https://www.zhihu.com/column/c_2058200382464078772)
- **交流群**：
  - XSTAR 官方 QQ 群：[579158551](https://qm.qq.com/q/eYZGJ5msJa)
  - XBOOT 官方 QQ 群：[658250248](https://jq.qq.com/?_wv=1027&k=5BOkXYO)
