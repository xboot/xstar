# Quick Start

Build and run your first XSTAR project in just a few minutes.

## Requirements

- **Operating System**: Linux (Ubuntu 24.04 or later recommended)
- **Basic Tools**: gcc, make, git
- **Graphical Version (Optional)**: SDL3 (only required by `x64-linux-sdl-helloworld`)

```bash
sudo apt-get install build-essential make git
```

## Build and Run

The minimal project `x64-linux-helloworld` has no external dependencies. After building, it enters an interactive Shell:

```bash
# From the repository root
make x64-linux-helloworld/xstar.defconfig
make -j$(nproc)

# Run (enters the XSTAR Shell)
./projects/x64-linux-helloworld/output/xstar
```

## Try the Graphical UI (Optional)

The SDL window version requires SDL3 to be installed first:

```bash
sudo apt-get install libsdl3-dev
make x64-linux-sdl-helloworld/xstar.defconfig
make -j$(nproc)
./projects/x64-linux-sdl-helloworld/output/xstar
```

## Commands to Try After Launch

Once in the Shell, you can run built-in commands:

```bash
help              # List all commands
version           # Show version info
ls /              # List the root directory
ls /kobj/device/  # Show registered devices
date              # Show date and time
echo hello        # Echo
```

## Cross-Compiling for Other Platforms

The toolchain prefix is preset in each project's defconfig — just switch projects. Install the required cross-toolchain for your platform:

```bash
# ARM64 Linux
sudo apt-get install gcc-aarch64-linux-gnu
make arm64-linux-helloworld/xstar.defconfig
make -j$(nproc)

# ARM32 baremetal (e.g. T113S3)
sudo apt-get install gcc-arm-linux-gnueabihf
make arm32-baremetal-t113s3/xstar.defconfig
make -j$(nproc)

# RISC-V32 Linux (e.g. V821)
make riscv32-linux-v821-helloworld/xstar.defconfig
make -j$(nproc)
```

## Common make Commands

| Command | Description |
|---------|-------------|
| `make <project>/xstar.defconfig` | Apply the project's default configuration |
| `make` | Build |
| `make -j$(nproc)` | Parallel build (faster) |
| `make menuconfig` | Interactive configuration menu |
| `make clean` | Clean build artifacts, keep configuration |
| `make distclean` | Clean everything, including configuration files |

You can also work inside a project directory (the Makefile delegates to the root directory):

```bash
cd projects/x64-linux-sdl-helloworld
make distclean
make defconfig
make
```

## Get Help

- **Source Repository**: [GitHub](https://github.com/xboot/xstar)
- **Mirror Repository**: [Gitee](https://gitee.com/xboot/xstar)
- **Further Reading**: [Zhihu Column](https://www.zhihu.com/column/c_2058200382464078772)
- **Discussion Groups**:
  - XSTAR Official QQ Group: [579158551](https://qm.qq.com/q/eYZGJ5msJa)
  - XBOOT Official QQ Group: [658250248](https://jq.qq.com/?_wv=1027&k=5BOkXYO)
