# 音频输出 (audiosink)

音频输出抽象，支持多种输出目的。

## 结构体

```c
struct audio_sink_t {
    struct audio_filter_t * filter;
    void (*write)(struct audio_sink_t * s, struct audio_frame_t * af);
    int (*ioctl)(struct audio_sink_t * s, const char * cmd, void * arg);
    void (*destroy)(struct audio_sink_t * s);
    void * priv;
};
```

## 创建函数

| 函数 | 说明 |
|------|------|
| `audio_sink_alloc()` | 分配空 Sink |
| `audio_sink_alloc_from_playback(name, rate, channel)` | 硬件播放输出 |
| `audio_sink_alloc_amplitude(period)` | 幅度包络提取 |
| `audio_sink_alloc_spectrum(period)` | FFT 频谱分析 |
| `audio_sink_alloc_vad(start, end)` | 语音活动检测 |
| `audio_sink_alloc_afsk(mark, space, rate, bit)` | AFSK 调制输出 |

## 控制函数

| 函数 | 说明 |
|------|------|
| `audio_sink_write(s, af)` | 写入音频帧 |
| `audio_sink_ioctl(s, cmd, arg)` | 设备控制 |
| `audio_sink_get/set_volume(s, vol)` | 获取/设置音量 |
| `audio_sink_free(s)` | 释放 Sink |

## ioctl 命令 {#ioctl}

通用 ioctl 命令（所有 sink 都应支持）：

| 命令字符串 | shash | arg 类型 | 说明 |
|----------|-------|---------|------|
| `"audio-sink-get-volume"` | `0xe04cfa2b` | `int *` | 获取音量 [0, 1000] |
| `"audio-sink-set-volume"` | `0x10cbc7b7` | `int *` | 设置音量 [0, 1000] |

`audio_sink_get_volume()` / `audio_sink_set_volume()` 即是上述 ioctl 的便捷封装。

特定 sink 的扩展命令示例：

| sink 类型 | 命令字符串 | 说明 |
|----------|----------|------|
| `spectrum` | `"spectrum-get-bins"` | 获取最近一次 FFT 频谱数组 |
| `amplitude` | `"amplitude-get-value"` | 获取当前振幅包络值 |
| `vad` | `"vad-get-state"` | 获取语音活动检测状态 |

具体参数与返回值参见各 sink 的源码实现。

## 数据流约定

调用 `audio_sink_write(s, af)` 时框架内部会自动执行：

```c
s->write(s, audio_filter_process(s->filter, af));
```

即写入的帧会先经过 sink 自带的 `filter` 后处理。例如 `sink-playback` 内置 `resample+reshape` filter 实现自动格式适配。详见 [音频效果](./audio-effect) 与 [典型使用示例](./usage-examples)。
