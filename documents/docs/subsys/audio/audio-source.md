# 音频源 (audiosource)

音频输入抽象，支持多种输入来源。

## 结构体

```c
struct audio_source_t {
    struct audio_filter_t * filter;
    int (*seek)(struct audio_source_t * s, int offset);
    int (*tell)(struct audio_source_t * s);
    int (*length)(struct audio_source_t * s);
    struct audio_frame_t * (*read)(struct audio_source_t * s);
    int (*ioctl)(struct audio_source_t * s, const char * cmd, void * arg);
    void (*destroy)(struct audio_source_t * s);
    void * priv;
};
```

## 创建函数

| 函数 | 说明 |
|------|------|
| `audio_source_alloc()` | 分配空 Source |
| `audio_source_alloc_from_xfs(ctx, filename)` | 从 XFS 文件读取（WAV/QOA） |
| `audio_source_alloc_from_capture(name, rate, channel)` | 从麦克风捕获 |
| `audio_source_alloc_from_memory(rate, channel, depth, mem, len)` | 从内存 PCM 数据 |
| `audio_source_alloc_afsk(mark, space, rate, bit, packet, len)` | AFSK 调制解调 |
| `audio_source_alloc_tone(type, rate, channel, freq, ms)` | 波形发生器 |
| `audio_source_alloc_noise(rate, channel)` | 白噪声发生器 |
| `audio_source_alloc_custom(rate, channel, cb, data)` | 用户自定义回调 |
| `audio_source_alloc_from_mixer(mixer)` | 将混音器包装为 Source |

## 控制函数

| 函数 | 说明 |
|------|------|
| `audio_source_read(s)` | 读取音频帧 |
| `audio_source_seek(s, offset)` | 跳转到指定位置 |
| `audio_source_tell(s)` | 获取当前位置 |
| `audio_source_length(s)` | 获取总长度 |
| `audio_source_get/set_volume(s, vol)` | 获取/设置音量 |
| `audio_source_free(s)` | 释放 Source |

## 音调类型

`audio_source_alloc_tone` 支持以下波形：

| 类型 | 说明 |
|------|------|
| `"sine"` | 正弦波 |
| `"square"` | 方波 |
| `"triangle"` | 三角波 |
| `"sawtooth"` | 锯齿波 |

## ioctl 命令 {#ioctl}

通用 ioctl 命令（所有 source 都应支持）：

| 命令字符串 | shash | arg 类型 | 说明 |
|----------|-------|---------|------|
| `"audio-source-get-volume"` | — | `int *` | 获取音量 [0, 1000] |
| `"audio-source-set-volume"` | — | `int *` | 设置音量 [0, 1000] |

`audio_source_get_volume()` / `audio_source_set_volume()` 即是上述 ioctl 的便捷封装。具体 source 可扩展自有命令，详见各 source 实现。

## 数据流约定

调用 `audio_source_read(s)` 时框架内部会自动执行：

```c
return audio_filter_process(s->filter, s->read(s));
```

即 source 自带的 `filter` 会自动作用于读取出的帧。可通过 `audio_source_filter_apply()` 替换或新增 filter。详见 [音频效果](./audio-effect) 与 [典型使用示例](./usage-examples)。
