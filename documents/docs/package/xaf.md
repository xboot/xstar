# xaf

XAF (X Animation Format) — 基于 Cinepak 编码的轻量级视频容器格式。解码器纯 C99 实现，约 540 行，零外部依赖。

## 二进制格式

所有多字节整数采用小端序 (Little-Endian)。

### 文件头 (16 bytes)

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0 | 4 | magic | `XAF\0` |
| 4 | 2 | width | 帧宽度 (LE16) |
| 6 | 2 | height | 帧高度 (LE16) |
| 8 | 4 | nframes | 总帧数 (LE32) |
| 12 | 1 | fps | 帧率 (1-255) |
| 13 | 3 | reserved | `0x000000` |

### 帧数据

每帧: `[data_len:U32][Cinepak 帧数据]`

Cinepak 帧数据为 ffmpeg Cinepak 编码器输出的原始码流（含 10 字节帧头 + strip 数据），每帧均为独立可解码的关键帧或帧间差分帧。

## 编码工具

使用 Python 脚本将任意视频转换为 XAF 格式。

### ffmpeg2xaf.py

通过 ffmpeg 子进程调用 Cinepak 编码器，以 AVI 为中间容器提取帧数据。

```bash
tools/ffmpeg2xaf.py video.mp4
tools/ffmpeg2xaf.py video.mp4 -o output.xaf -f 24 -w 320 -H 240
```

依赖：ffmpeg

### pyav2xaf.py

通过 PyAV 直接调用 Cinepak 编码器，无中间容器，无 ffmpeg 子进程。

```bash
tools/pyav2xaf.py video.mp4
tools/pyav2xaf.py video.mp4 -o output.xaf -f 24 -w 320 -H 240
```

依赖：PyAV (`pip install av`)

### 编码选项

| 选项 | 说明 |
|------|------|
| `-o, --output` | 输出文件（默认 `<输入>.xaf`） |
| `-f, --fps` | 帧率 |
| `-w, --width` | 输出宽度 |
| `-H, --height` | 输出高度 |

## 数据结构

```c
struct xaf_t {
    uint16_t width;      /* 视频宽度 */
    uint16_t height;     /* 视频高度 */
    uint32_t nframes;    /* 总帧数 */
    uint8_t fps;         /* 帧率 */
    uint32_t findex;     /* 当前帧索引 (0-based) */
    uint8_t * data;      /* 原始数据 */
    uint32_t size;       /* 数据大小 */
    uint32_t pos;        /* 读取位置 */
    uint32_t * frame;    /* 解码帧缓冲 (BGRA) */
    uint32_t fsize;      /* 帧缓冲大小 */
    struct cinepak_ctx_t cinepak; /* Cinepak 解码上下文 */
};
```

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

| 函数 | 说明 |
|------|------|
| `xaf_alloc(data, size)` | 从内存创建解码器，失败返回 NULL |
| `xaf_free(ctx)` | 释放解码器 |
| `xaf_next(ctx)` | 解码下一帧，返回内部帧缓冲指针（width × height BGRA 像素，行 stride = width × 4 字节），帧结束或失败返回 NULL |
| `xaf_reset(ctx)` | 重置到第一帧 |
| `xaf_get_width(ctx)` | 帧宽度 |
| `xaf_get_height(ctx)` | 帧高度 |
| `xaf_get_nframes(ctx)` | 总帧数 |
| `xaf_get_fps(ctx)` | 帧率 |
| `xaf_get_findex(ctx)` | 当前帧号 (0-based) |

## Shell 命令

`xaf` 命令用于在 XSTAR 中播放 XAF 视频文件。

```
xaf <file> [-m=none|contain|cover|fill] [-c=color] [-f=framebuffer] [-i=input] [-lock] [-hide] [-loop]
```

### 选项

| 选项 | 说明 |
|------|------|
| `-m=<mode>` | 显示模式：`none`（原始大小，居中显示，不缩放）、`contain`（等比缩放至完全可见，短边适配，可能留黑边）、`cover`（等比缩放至完全覆盖，长边适配，可能裁剪）、`fill`（拉伸填满窗口，不保持宽高比） |
| `-c=<color>` | 背景色（十六进制，如 `#000000`） |
| `-f=<fb>` | 目标帧缓冲设备 |
| `-i=<input>` | 输入设备 |
| `-lock` | 锁定，不响应键盘事件 |
| `-hide` | 隐藏进度条 |
| `-loop` | 循环播放 |

### 快捷键

| 按键 | 说明 |
|------|------|
| `Space` | 切换显示模式 (none → contain → cover → fill) |
| `Home` | 退出播放 |

## 嵌入式集成

仅需复制 `src/xaf.h` 和 `src/xaf.c` 两个文件到项目中。解码器为纯 C99，零外部依赖，`xaf.h` 自包含（已包含所需标准库头文件）。
