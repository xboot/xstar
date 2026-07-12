# XSTAR

A portable embedded system framework for writing cross-platform firmware and applications that run unchanged on baremetal, rtos, linux, windows and other OS.

[![GitHub](https://img.shields.io/badge/GitHub-xboot%2Fxstar-181717?logo=github)](https://github.com/xboot/xstar)
[![Gitee](https://img.shields.io/badge/Gitee-xboot%2Fxstar-C71D23?logo=gitee)](https://gitee.com/xboot/xstar)
[![Docs](https://img.shields.io/badge/Docs-xstar.xboot.org-blue?logo=readthedocs)](https://xstar.xboot.org/)
[![Zhihu](https://img.shields.io/badge/Zhihu-Column-0084FF?logo=zhihu)](https://www.zhihu.com/column/c_2058200382464078772)

---

## Quick Start

Requirements: GCC toolchain (multi-arch), GNU Make, Linux or Windows host.

```bash
# Apply a project config and build (from repo root)
make x64-linux-sdl-helloworld/xstar.defconfig
make

# Common targets
make clean          # remove build artifacts, keep config
make distclean      # remove everything, including config
make menuconfig     # interactive configuration
```

Build from a project directory instead:

```bash
cd projects/x64-linux-sdl-helloworld
make defconfig
make
```

---

## Architecture

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

---

## Platforms

| Arch | Targets |
|------|--------|
| ARM32 | baremetal: rk3506, rv1103, rv1106, t113s3 · linux: helloworld |
| ARM64 | baremetal: t527 · linux: helloworld |
| RISC-V32 | baremetal: v821 · freertos: v821 · linux: v821 |
| RISC-V64 | baremetal: f133 |
| x64 | linux: helloworld, sdl-helloworld, sdl-xiaozhi · windows: sdl-helloworld |

See `projects/` for the full list and configs.

---

## Links

- **Project** (GitHub): <https://github.com/xboot/xstar>
- **Mirror** (Gitee): <https://gitee.com/xboot/xstar>
- **Documentation**: <https://xstar.xboot.org/>
- **Zhihu Column**: <https://www.zhihu.com/column/c_2058200382464078772>

---

## Discussion Groups
- XSTAR Official Tencent QQ Group: [579158551](https://qm.qq.com/q/eYZGJ5msJa)
- XBOOT Official Tencent QQ Group: [658250248](https://jq.qq.com/?_wv=1027&k=5BOkXYO)

---

## License

This library is free software; you can redistribute it and or modify it under the terms of the MIT license. See [MIT License](LICENSE) for details.
