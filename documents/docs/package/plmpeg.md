# plmpeg

MPEG1 (VCD) 视频播放器。基于 [PL_MPEG](https://github.com/phoboslab/pl_mpeg)（Dominic Szablewski 的 MIT 单头文件 MPEG1 解码器）移植到 XSTAR，使用 XOS API 替代标准 C 库调用，支持 XFS 文件系统读取。

## 配置

```bash
# 编码 MPEG1 文件（推荐编码器参数）
ffmpeg -i input.mp4 -c:v mpeg1video -q:v 0 -c:a libtwolame -b:a 224k -f mpeg output.mpg
```

## XSTAR 适配

- `PLM_NO_STDIO` 定义（使用 XFS 替代标准文件 I/O）
- `PLM_MALLOC`/`PLM_FREE`/`PLM_REALLOC` 映射到 `xos_mem_malloc`/`xos_mem_free`/`xos_mem_realloc`
- 新增 XFS 创建 API：`plm_create_with_xfs_filename()`、`plm_buffer_create_with_xfs_filename()`、`plm_buffer_create_with_xfs_file()`

## Shell 命令

`plmpeg` 命令用于播放 MPEG1 视频文件，支持视频（帧缓冲）和音频（音频子系统）同步输出。

```
plmpeg <file> [-m=none|contain|cover|fill] [-c=color] [-f=framebuffer] [-i=input] [-p=playback] [-lock] [-hide] [-loop]
```

### 选项

| 选项 | 说明 |
|------|------|
| `-m=<mode>` | 显示模式：`none`（原始大小，居中显示，不缩放）、`contain`（等比缩放至完全可见，短边适配，可能留黑边）、`cover`（等比缩放至完全覆盖，长边适配，可能裁剪）、`fill`（拉伸填满窗口，不保持宽高比） |
| `-c=<color>` | 背景色（十六进制，如 `#000000`） |
| `-f=<fb>` | 目标帧缓冲设备 |
| `-i=<input>` | 输入设备 |
| `-p=<playback>` | 音频播放设备 |
| `-lock` | 锁定，不响应键盘 |
| `-hide` | 隐藏进度条 |
| `-loop` | 循环播放 |

### 快捷键

| 按键 | 说明 |
|------|------|
| `Space` | 切换显示模式 |
| `Home` | 退出播放 |
| `←` / `→` | 后退/快进 3 秒 |
| `↑` / `↓` | 音量 +100/-100 |
| 鼠标拖拽 | 拖动进度条 |

## 数据结构

```c
/* 解码视频帧 */
typedef struct {
    double time;          /* PTS 时间戳 */
    unsigned int width;   /* 显示宽度 */
    unsigned int height;  /* 显示高度 */
    plm_plane_t y;        /* 亮度平面 */
    plm_plane_t cr;       /* 色度 Cr 平面 */
    plm_plane_t cb;       /* 色度 Cb 平面 */
} plm_frame_t;

/* 解码音频样本 */
#define PLM_AUDIO_SAMPLES_PER_FRAME 1152

typedef struct {
    double time;                  /* PTS 时间戳 */
    unsigned int count;           /* 样本数 */
    float interleaved[...];       /* 交织立体声样本 */
} plm_samples_t;

/* 解复用数据包 */
typedef struct {
    int type;           /* 包类型 */
    double pts;         /* PTS 时间戳 */
    size_t length;      /* 数据长度 */
    uint8_t *data;      /* 数据指针 */
} plm_packet_t;
```

## API 概述

### 高级接口 (plm_*)

| 函数 | 说明 |
|------|------|
| `plm_create_with_filename(name)` | 从文件名创建 |
| `plm_create_with_file(fh, close_when_done)` | 从文件句柄创建 |
| `plm_create_with_xfs_filename(ctx, name)` | 从 XFS 路径创建 |
| `plm_create_with_memory(bytes, len, free_when_done)` | 从内存创建 |
| `plm_create_with_buffer(buffer, destroy_when_done)` | 从缓冲创建 |
| `plm_destroy(self)` | 销毁 |
| `plm_has_headers(self)` | 是否解析到头信息 |
| `plm_probe(self, probesize)` | 探测实际流数量 |
| `plm_get_width/height/framerate(self)` | 获取视频参数 |
| `plm_get_samplerate(self)` | 获取音频采样率 |
| `plm_get/loop(self)` | 获取/设置循环 |
| `plm_get_duration(self)` | 获取视频时长 |
| `plm_get_time(self)` | 获取当前时间 |
| `plm_set_video/audio_decode_callback(self, cb, user)` | 设置解码回调 |
| `plm_decode(self, seconds)` | 解码指定时长（驱动主循环） |
| `plm_decode_video(self)` | 解码一帧视频 |
| `plm_decode_audio(self)` | 解码一帧音频 |
| `plm_seek(self, time, seek_exact)` | 跳转到指定时间 |
| `plm_seek_frame(self, time, seek_exact)` | 跳转并返回帧 |
| `plm_rewind(self)` | 重置到开头 |

### 缓冲接口 (plm_buffer_*)

| 函数 | 说明 |
|------|------|
| `plm_buffer_create_with_filename(name)` | 从文件名创建 |
| `plm_buffer_create_with_file(fh, close_when_done)` | 从文件句柄创建 |
| `plm_buffer_create_with_xfs_filename(ctx, name)` | 从 XFS 路径创建 |
| `plm_buffer_create_with_xfs_file(file)` | 从 XFS 文件句柄创建 |
| `plm_buffer_create_with_memory(bytes, len, free_when_done)` | 从内存创建（固定） |
| `plm_buffer_create_with_capacity(capacity)` | 创建环形缓冲 |
| `plm_buffer_create_for_appending(capacity)` | 创建追加缓冲（可回溯） |
| `plm_buffer_write(self, bytes, len)` | 写入数据 |
| `plm_buffer_signal_end(self)` | 标记数据结束 |
| `plm_buffer_get_size(self)` | 获取总大小 |
| `plm_buffer_get_remaining(self)` | 获取剩余字节 |
| `plm_buffer_has_ended(self)` | 是否已结束 |

### 解复用接口 (plm_demux_*)

| 函数 | 说明 |
|------|------|
| `plm_demux_create(buffer, destroy_when_done)` | 创建解复用器 |
| `plm_demux_destroy(self)` | 销毁 |
| `plm_demux_has_headers(self)` | 是否解析到头信息 |
| `plm_demux_probe(self, probesize)` | 探测流数量 |
| `plm_demux_get_num_video/audio_streams(self)` | 获取流数量 |
| `plm_demux_decode(self)` | 解码下一数据包 |
| `plm_demux_seek(self, time, type, force_intra)` | 按时间跳转 |
| `plm_demux_get_duration(self, type)` | 获取指定流时长 |

### 视频解码接口 (plm_video_*)

| 函数 | 说明 |
|------|------|
| `plm_video_create_with_buffer(buffer, destroy_when_done)` | 创建视频解码器 |
| `plm_video_destroy(self)` | 销毁 |
| `plm_video_has_header(self)` | 是否有序列头 |
| `plm_video_get_width/height/framerate(self)` | 获取视频参数 |
| `plm_video_decode(self)` | 解码一帧 |
| `plm_frame_to_rgb/bgr/rgba/bgra/argb/abgr(frame, dest, stride)` | YCrCb → RGB 转换 |

### 音频解码接口 (plm_audio_*)

| 函数 | 说明 |
|------|------|
| `plm_audio_create_with_buffer(buffer, destroy_when_done)` | 创建音频解码器 |
| `plm_audio_destroy(self)` | 销毁 |
| `plm_audio_has_header(self)` | 是否有帧头 |
| `plm_audio_get_samplerate(self)` | 获取采样率 |
| `plm_audio_decode(self)` | 解码一帧音频 |

## 播放流程

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
