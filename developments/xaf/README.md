# XAF - X Animation Format

基于 Cinepak 编码的轻量级视频容器格式，仅包含解码器（`xaf.c` ~470 行 + `xaf.h` ~70 行，共 ~540 行 C99，零外部依赖），编码全部交给 ffmpeg 完成。

## 二进制格式

所有多字节整数采用小端序 (Little-Endian)。

### 文件头 (16 bytes, packed)

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0 | 4 | magic | `XAF\0` |
| 4 | 2 | width | LE16 |
| 6 | 2 | height | LE16 |
| 8 | 4 | nframes | LE32 |
| 12 | 1 | fps | 1-255 |
| 13 | 3 | reserved | `0x000000` |

### 帧数据

每帧: `[data_len:U32][Cinepak 帧数据]`

其中 Cinepak 帧数据为 ffmpeg Cinepak 编码器输出的原始码流（含 10 字节帧头 + strip 数据），每帧均为独立可解码的关键帧或帧间差分帧。

## 文件结构

```
├── src/
│   ├── xaf.h              公共 API（1 个头文件）
│   ├── xaf.c              解码器 + 内嵌 Cinepak 解码器（~470 行，零依赖）
│   ├── xaf.mk             构建包含文件
├── projects/linux/
│   ├── main.c             SDL2 播放器（Linux）
│   ├── Makefile           构建脚本
│   └── output/            编译产物目录
├── projects/windows/
│   ├── main.c             SDL2 播放器（Windows）
│   ├── Makefile           mingw 交叉编译脚本
│   ├── xaf.res            Windows 资源文件
│   └── SDL2-2.30.8/       Windows 预编译 SDL2
├── tools/
│   ├── ffmpeg2xaf.py      Python 脚本：任意视频 → XAF（调用 ffmpeg）
│   └── pyav2xaf.py        Python 脚本：任意视频 → XAF（调用 PyAV）
├── tests/
│   ├── test.mp4           测试视频
│   └── test.xaf           测试 XAF 文件
├── Makefile               顶层构建入口
└── LICENSE                MIT
```

## 编译

```bash
make                     # 同时构建 Linux 和 Windows（需要 mingw-w64）
make -C projects/linux   # 仅构建 Linux 播放器（只需 SDL2）
```

| 二进制 | 源码 | 依赖 |
|--------|------|------|
| `projects/linux/output/xaf` | `projects/linux/main.c` + `src/xaf.c` | SDL2 |
| `projects/windows/output/xaf.exe` | `projects/windows/main.c` + `src/xaf.c` | mingw-w64 + SDL2

## 使用方法

### 编码 — ffmpeg2xaf.py

通过 ffmpeg 子进程调用 Cinepak 编码器，以 AVI 为中间容器提取帧数据。

```bash
tools/ffmpeg2xaf.py video.mp4

# 指定输出 / 帧率 / 分辨率
tools/ffmpeg2xaf.py video.mp4 -o output.xaf -f 24 -w 320 -H 240
```

依赖：ffmpeg

### 编码 — pyav2xaf.py

通过 PyAV 直接调用 Cinepak 编码器，无中间容器，无 ffmpeg 子进程。

```bash
tools/pyav2xaf.py video.mp4

# 指定输出 / 帧率 / 分辨率
tools/pyav2xaf.py video.mp4 -o output.xaf -f 24 -w 320 -H 240
```

依赖：PyAV (`pip install av`)

### 共同选项

| 选项 | 说明 |
|------|------|
| `-o, --output` | 输出文件（默认 `<输入>.xaf`） |
| `-f, --fps` | 帧率 |
| `-w, --width` | 输出宽度（可单独使用） |
| `-H, --height` | 输出高度（可单独使用） |

### SDL2 播放器

```bash
projects/linux/output/xaf video.xaf
```

**操作键位：**
Space 暂停/继续，R 重新开始，Q/Esc 退出。

**播放器特性：**
- 使用 streaming texture 直接更新帧数据，窗口可自由缩放
- 最近邻插值，窗口标题显示文件名、当前帧/总帧数及暂停状态
- 播放结束后自动循环

## API

```c
#include <xaf.h>

/* 从内存中的 XAF 数据创建解码器 */
struct xaf_t *ctx = xaf_alloc(data, data_size);
if (!ctx) { /* 格式错误 */ }

/* 读取元数据 */
uint16_t width   = xaf_get_width(ctx);
uint16_t height  = xaf_get_height(ctx);
uint32_t nframes = xaf_get_nframes(ctx);
uint8_t  fps     = xaf_get_fps(ctx);
uint32_t cur     = xaf_get_findex(ctx);

/* 逐帧解码 */
uint32_t *frame;
for (uint32_t i = 0; i < nframes; i++) {
    frame = xaf_next(ctx);
    if (!frame) break;
    /* 处理 frame（width * height 像素，stride = width * 4 bytes）... */
}

/* 重置到第一帧 */
xaf_reset(ctx);

/* 释放 */
xaf_free(ctx);
```

### API 一览

| 函数 | 说明 |
|------|------|
| `xaf_alloc(data, size)` | 从内存创建解码器，失败返回 NULL |
| `xaf_free(ctx)` | 释放解码器 |
| `xaf_next(ctx)` | 解码下一帧，返回内部帧缓冲区指针（width × height BGRA 像素，行 stride = width × 4 字节），帧结束或失败返回 NULL |
| `xaf_reset(ctx)` | 重置到第一帧 |
| `xaf_get_width(ctx)` | 帧宽度 |
| `xaf_get_height(ctx)` | 帧高度 |
| `xaf_get_nframes(ctx)` | 总帧数 |
| `xaf_get_fps(ctx)` | 帧率 |
| `xaf_get_findex(ctx)` | 当前帧号 (0-based) |

## 嵌入式集成

仅需复制 `src/xaf.h` 和 `src/xaf.c` 两个文件。解码器为纯 C99，零外部依赖，`xaf.h` 自包含（已包含所需标准库头文件）。
