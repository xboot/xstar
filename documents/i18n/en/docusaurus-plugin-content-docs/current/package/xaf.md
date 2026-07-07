# xaf

XAF (X Animation Format) — A lightweight video container format based on Cinepak encoding. Decoder is pure C99, ~540 lines, zero external dependencies.

## Binary Format

All multi-byte integers use Little-Endian.

### File Header (16 bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 4 | magic | `XAF\0` |
| 4 | 2 | width | Frame width (LE16) |
| 6 | 2 | height | Frame height (LE16) |
| 8 | 4 | nframes | Total frame count (LE32) |
| 12 | 1 | fps | Frame rate (1-255) |
| 13 | 3 | reserved | `0x000000` |

### Frame Data

Per frame: `[data_len:U32][Cinepak frame data]`

The Cinepak frame data is the raw bitstream output from the ffmpeg Cinepak encoder (including 10-byte frame header + strip data), each frame is an independently decodable key frame or inter-frame delta frame.

## Encoding Tools

Use Python scripts to convert any video to XAF format.

### ffmpeg2xaf.py

Invokes the Cinepak encoder via an ffmpeg subprocess, using AVI as the intermediate container to extract frame data.

```bash
tools/ffmpeg2xaf.py video.mp4
tools/ffmpeg2xaf.py video.mp4 -o output.xaf -f 24 -w 320 -H 240
```

Dependency: ffmpeg

### pyav2xaf.py

Directly invokes the Cinepak encoder via PyAV, no intermediate container, no ffmpeg subprocess.

```bash
tools/pyav2xaf.py video.mp4
tools/pyav2xaf.py video.mp4 -o output.xaf -f 24 -w 320 -H 240
```

Dependency: PyAV (`pip install av`)

### Encoding Options

| Option | Description |
|--------|-------------|
| `-o, --output` | Output file (default `<input>.xaf`) |
| `-f, --fps` | Frame rate |
| `-w, --width` | Output width |
| `-H, --height` | Output height |

## Data Structure

```c
struct xaf_t {
    uint16_t width;      /* Video width */
    uint16_t height;     /* Video height */
    uint32_t nframes;    /* Total frame count */
    uint8_t fps;         /* Frame rate */
    uint32_t findex;     /* Current frame index (0-based) */
    uint8_t * data;      /* Raw data */
    uint32_t size;       /* Data size */
    uint32_t pos;        /* Read position */
    uint32_t * frame;    /* Decoded frame buffer (BGRA) */
    uint32_t fsize;      /* Frame buffer size */
    struct cinepak_ctx_t cinepak; /* Cinepak decoding context */
};
```

## API

```c
#include <xaf.h>

/* Create decoder from in-memory XAF data */
struct xaf_t *ctx = xaf_alloc(data, data_size);
if (!ctx) { /* Format error */ }

/* Read metadata */
uint16_t width   = xaf_get_width(ctx);
uint16_t height  = xaf_get_height(ctx);
uint32_t nframes = xaf_get_nframes(ctx);
uint8_t  fps     = xaf_get_fps(ctx);
uint32_t cur     = xaf_get_findex(ctx);

/* Decode frame by frame */
uint32_t *frame;
for (uint32_t i = 0; i < nframes; i++) {
    frame = xaf_next(ctx);
    if (!frame) break;
    /* Process frame (width * height pixels, stride = width * 4 bytes) ... */
}

/* Reset to first frame */
xaf_reset(ctx);

/* Free */
xaf_free(ctx);
```

| Function | Description |
|----------|-------------|
| `xaf_alloc(data, size)` | Create decoder from memory, returns NULL on failure |
| `xaf_free(ctx)` | Free decoder |
| `xaf_next(ctx)` | Decode next frame, returns internal frame buffer pointer (width × height BGRA pixels, row stride = width × 4 bytes), returns NULL at end of frames or on failure |
| `xaf_reset(ctx)` | Reset to first frame |
| `xaf_get_width(ctx)` | Frame width |
| `xaf_get_height(ctx)` | Frame height |
| `xaf_get_nframes(ctx)` | Total frame count |
| `xaf_get_fps(ctx)` | Frame rate |
| `xaf_get_findex(ctx)` | Current frame number (0-based) |

## Shell Command

The `xaf` command is used to play XAF video files in XSTAR.

```
xaf <file> [-m=none|contain|cover|fill] [-c=color] [-f=framebuffer] [-i=input] [-lock] [-hide] [-loop]
```

### Options

| Option | Description |
|--------|-------------|
| `-m=<mode>` | Display mode: `none` (original size, centered, no scaling), `contain` (scale proportionally to fit entirely, shorter side fits, may have letterboxing), `cover` (scale proportionally to cover entirely, longer side fits, may crop), `fill` (stretch to fill window, aspect ratio not preserved) |
| `-c=<color>` | Background color (hexadecimal, e.g. `#000000`) |
| `-f=<fb>` | Target framebuffer device |
| `-i=<input>` | Input device |
| `-lock` | Lock, do not respond to keyboard events |
| `-hide` | Hide progress bar |
| `-loop` | Loop playback |

### Shortcuts

| Key | Description |
|-----|-------------|
| `Space` | Toggle display mode (none → contain → cover → fill) |
| `Home` | Exit playback |

## Embedded Integration

Simply copy the two files `src/xaf.h` and `src/xaf.c` into your project. The decoder is pure C99, zero external dependencies, `xaf.h` is self-contained (includes required standard library headers).
