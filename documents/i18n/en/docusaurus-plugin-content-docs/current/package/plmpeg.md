# plmpeg

MPEG1 (VCD) video player. Ported from [PL_MPEG](https://github.com/phoboslab/pl_mpeg) (Dominic Szablewski's MIT single-header MPEG1 decoder) to XSTAR, using XOS API instead of standard C library calls, with XFS filesystem read support.

## Configuration

```bash
# Encode MPEG1 file (recommended encoder parameters)
ffmpeg -i input.mp4 -c:v mpeg1video -q:v 0 -c:a libtwolame -b:a 224k -f mpeg output.mpg
```

## XSTAR Adaptation

- `PLM_NO_STDIO` defined (uses XFS instead of standard file I/O)
- `PLM_MALLOC`/`PLM_FREE`/`PLM_REALLOC` mapped to `xos_mem_malloc`/`xos_mem_free`/`xos_mem_realloc`
- New XFS creation APIs: `plm_create_with_xfs_filename()`, `plm_buffer_create_with_xfs_filename()`, `plm_buffer_create_with_xfs_file()`

## Shell Command

The `plmpeg` command is used to play MPEG1 video files, supporting synchronized video (framebuffer) and audio (audio subsystem) output.

```
plmpeg <file> [-m=none|contain|cover|fill] [-c=color] [-f=framebuffer] [-i=input] [-p=playback] [-lock] [-hide] [-loop]
```

### Options

| Option | Description |
|--------|-------------|
| `-m=<mode>` | Display mode: `none` (original size, centered, no scaling), `contain` (scale proportionally to fit entirely, shorter side fits, may have letterboxing), `cover` (scale proportionally to cover entirely, longer side fits, may crop), `fill` (stretch to fill window, aspect ratio not preserved) |
| `-c=<color>` | Background color (hexadecimal, e.g. `#000000`) |
| `-f=<fb>` | Target framebuffer device |
| `-i=<input>` | Input device |
| `-p=<playback>` | Audio playback device |
| `-lock` | Lock, do not respond to keyboard |
| `-hide` | Hide progress bar |
| `-loop` | Loop playback |

### Shortcuts

| Key | Description |
|-----|-------------|
| `Space` | Toggle display mode |
| `Home` | Exit playback |
| `←` / `→` | Rewind / fast forward 3 seconds |
| `↑` / `↓` | Volume +100 / -100 |
| Mouse drag | Drag progress bar |

## Data Structures

```c
/* Decoded video frame */
typedef struct {
    double time;          /* PTS timestamp */
    unsigned int width;   /* Display width */
    unsigned int height;  /* Display height */
    plm_plane_t y;        /* Luma plane */
    plm_plane_t cr;       /* Chroma Cr plane */
    plm_plane_t cb;       /* Chroma Cb plane */
} plm_frame_t;

/* Decoded audio samples */
#define PLM_AUDIO_SAMPLES_PER_FRAME 1152

typedef struct {
    double time;                  /* PTS timestamp */
    unsigned int count;           /* Sample count */
    float interleaved[...];       /* Interleaved stereo samples */
} plm_samples_t;

/* Demuxed packet */
typedef struct {
    int type;           /* Packet type */
    double pts;         /* PTS timestamp */
    size_t length;      /* Data length */
    uint8_t *data;      /* Data pointer */
} plm_packet_t;
```

## API Overview

### High-level Interface (plm_*)

| Function | Description |
|----------|-------------|
| `plm_create_with_filename(name)` | Create from filename |
| `plm_create_with_file(fh, close_when_done)` | Create from file handle |
| `plm_create_with_xfs_filename(ctx, name)` | Create from XFS path |
| `plm_create_with_memory(bytes, len, free_when_done)` | Create from memory |
| `plm_create_with_buffer(buffer, destroy_when_done)` | Create from buffer |
| `plm_destroy(self)` | Destroy |
| `plm_has_headers(self)` | Whether headers have been parsed |
| `plm_probe(self, probesize)` | Probe actual stream count |
| `plm_get_width/height/framerate(self)` | Get video parameters |
| `plm_get_samplerate(self)` | Get audio sample rate |
| `plm_get/loop(self)` | Get/set loop |
| `plm_get_duration(self)` | Get video duration |
| `plm_get_time(self)` | Get current time |
| `plm_set_video/audio_decode_callback(self, cb, user)` | Set decode callback |
| `plm_decode(self, seconds)` | Decode for specified duration (drives main loop) |
| `plm_decode_video(self)` | Decode one video frame |
| `plm_decode_audio(self)` | Decode one audio frame |
| `plm_seek(self, time, seek_exact)` | Seek to specified time |
| `plm_seek_frame(self, time, seek_exact)` | Seek and return frame |
| `plm_rewind(self)` | Reset to beginning |

### Buffer Interface (plm_buffer_*)

| Function | Description |
|----------|-------------|
| `plm_buffer_create_with_filename(name)` | Create from filename |
| `plm_buffer_create_with_file(fh, close_when_done)` | Create from file handle |
| `plm_buffer_create_with_xfs_filename(ctx, name)` | Create from XFS path |
| `plm_buffer_create_with_xfs_file(file)` | Create from XFS file handle |
| `plm_buffer_create_with_memory(bytes, len, free_when_done)` | Create from memory (fixed) |
| `plm_buffer_create_with_capacity(capacity)` | Create ring buffer |
| `plm_buffer_create_for_appending(capacity)` | Create append buffer (backtrackable) |
| `plm_buffer_write(self, bytes, len)` | Write data |
| `plm_buffer_signal_end(self)` | Signal end of data |
| `plm_buffer_get_size(self)` | Get total size |
| `plm_buffer_get_remaining(self)` | Get remaining bytes |
| `plm_buffer_has_ended(self)` | Whether it has ended |

### Demux Interface (plm_demux_*)

| Function | Description |
|----------|-------------|
| `plm_demux_create(buffer, destroy_when_done)` | Create demuxer |
| `plm_demux_destroy(self)` | Destroy |
| `plm_demux_has_headers(self)` | Whether headers have been parsed |
| `plm_demux_probe(self, probesize)` | Probe stream count |
| `plm_demux_get_num_video/audio_streams(self)` | Get stream count |
| `plm_demux_decode(self)` | Decode next packet |
| `plm_demux_seek(self, time, type, force_intra)` | Seek by time |
| `plm_demux_get_duration(self, type)` | Get duration of specified stream |

### Video Decode Interface (plm_video_*)

| Function | Description |
|----------|-------------|
| `plm_video_create_with_buffer(buffer, destroy_when_done)` | Create video decoder |
| `plm_video_destroy(self)` | Destroy |
| `plm_video_has_header(self)` | Whether sequence header is present |
| `plm_video_get_width/height/framerate(self)` | Get video parameters |
| `plm_video_decode(self)` | Decode one frame |
| `plm_frame_to_rgb/bgr/rgba/bgra/argb/abgr(frame, dest, stride)` | YCrCb → RGB conversion |

### Audio Decode Interface (plm_audio_*)

| Function | Description |
|----------|-------------|
| `plm_audio_create_with_buffer(buffer, destroy_when_done)` | Create audio decoder |
| `plm_audio_destroy(self)` | Destroy |
| `plm_audio_has_header(self)` | Whether frame header is present |
| `plm_audio_get_samplerate(self)` | Get sample rate |
| `plm_audio_decode(self)` | Decode one audio frame |

## Playback Flow

```c
#define PL_MPEG_IMPLEMENTATION
#include <plmpeg.h>

plm_t *plm = plm_create_with_xfs_filename(ctx, "video.mpg");

plm_set_video_decode_callback(plm, on_video, user_data);
plm_set_audio_decode_callback(plm, on_audio, user_data);

while (!plm_has_ended(plm)) {
    double elapsed = now - last;
    plm_decode(plm, elapsed);
}

plm_destroy(plm);
```
