# XSTAR

A portable embedded system framework for writing cross-platform firmware and applications that run unchanged on baremetal, rtos, linux, windows and other OS.

[Quick Start](#quick-start) · [Features](#features) · [Platforms](#platforms) · [Project Layout](#project-layout) · [Docs](#docs)

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
make defconfig && make
```

---

## Features

- **Cross-platform XOS layer** — One `xos_environ_t` struct abstracts memory, DMA, I/O, filesystem, coroutines, threads, and mutexes. The same source compiles for baremetal, Linux, and FreeRTOS.
- **Assembly coroutines** — Context switching in hand-written assembly for ARM32/ARM64, RISC-V32/64, and x64, with full register save/restore and `yield` / `ns|us|ms` sleep.
- **JSON device tree** — Devices are declared in readable JSON (e.g. `romdisk/dtree/default.json`) instead of binary DTS, parsed via `dt_read_*` helpers.
- **Linux-style driver framework** — `driver`/`device` split with `probe/remove/suspend/resume` lifecycle, O(1) hash-table lookup, and auto-registration through `*_initcall` macros across 51 device types (UART, I2C, SPI, GPIO, ADC, framebuffer, audio, NAND, SD, …).
- **Kobj virtual filesystem** — A sysfs-like tree exposing every device/driver as a readable/writable node (e.g. `/kobj/device/framebuffer/fb.0/width`).
- **Pub/Sub events** — Topic-based, thread-safe messaging (`psub_publish` / `psub_subscribe`) for events like `device.add`, `driver.register`, `framebuffer.update`.
- **Graphics stack** — 32-bit ARGB with premultiplied alpha, software + G2D accelerated rendering, QOI/PNG/JPEG decoding, and dirty-rectangle optimization.
- **Tiered initcalls** — 10 linker-section levels (`pure → machine → core → postcore → driver → subsys → command → server → wboxtest → late`) drive deterministic boot ordering.
- **LibX toolbox** — Algorithms (FFT, Kalman, biquad, EWMA), data structures (hash, red-black tree, LRU, FIFO), crypto (AES, SHA, ECDSA, RC4, Base64), and parsers (JSON, DT, URI, UUID).

---

## Project Layout

```
xstar/
├── xstar/
│   ├── xos/        # Platform abstraction (baremetal / Linux / FreeRTOS)
│   ├── driver/     # 50+ device driver types
│   ├── kernel/     # audio, command, core, graphic, shell, time, vision, window, xfs
│   ├── libx/       # Algorithms, data structures, crypto, image utils
│   └── external/   # Vendored libraries (jpeg, libpng, zlib, ...)
├── packages/       # External packages (wboxtest, lvgl, libc, ...)
├── projects/       # Per-platform configs and romdisk
├── tools/          # Build tools (kconfig)
└── documents/      # Docusaurus documentation site
```

---

## Platforms

| Arch | Targets |
|------|--------|
| ARM32 | baremetal: rk3506, rv1103, rv1106, t113s3 · linux: helloworld |
| ARM64 | baremetal: t527 · linux: helloworld |
| RISC-V32 | baremetal: v821 · freertos: v821 · linux: v821 (+ xiaozhi variants) |
| RISC-V64 | baremetal: f133 |
| x64 | linux: helloworld, sdl-helloworld, sdl-xiaozhi · windows: sdl-helloworld |

See `projects/` for the full list and configs.

---

## Docs

Hosted docs are built with Docusaurus under `documents/`.

- [Build Guide](documents/docs/guide/build-guide.md)
- [Architecture Design](documents/docs/guide/architecture-design.md)
- [Development Guide](documents/docs/guide/development-guide.md)
- [Device Tree (JSON)](documents/docs/guide/device-tree.md)

---

## Discussion Group, Many Big Brother, Please Join Us
XSTAR Official Tencent QQ Group: [658250248](https://qm.qq.com/q/b4yjKSjjmo) (2000 people)

---

## License

MIT — see [LICENSE](LICENSE).
