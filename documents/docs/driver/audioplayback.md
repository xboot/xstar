# 音频播放 (audioplayback)

音频播放（扬声器输出）。

## 设备类型

`DEVICE_TYPE_AUDIOPLAYBACK`

## 结构体

```c
struct audio_playback_t {
    char * name;
    int (*start)(struct audio_playback_t * playback, int rate, int channel);
    int (*write)(struct audio_playback_t * playback, float * samples, int nsample);
    int (*stop)(struct audio_playback_t * playback);
    int (*ioctl)(struct audio_playback_t * playback, const char * cmd, void * arg);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_audio_playback(name)` | 按名称查找音频播放设备 |
| `register_audio_playback(playback, drv)` | 注册音频播放设备 |
| `unregister_audio_playback(playback)` | 注销音频播放设备 |
| `audio_playback_start/stop(playback, ...)` | 启动/停止播放 |
| `audio_playback_write(playback, samples, nsample)` | 写入音频样本 |
| `audio_playback_get/set_volume(playback)` | 获取/设置音量 |

## 说明

音频播放（扬声器输出）接口。以指定采样率和通道数播放音频样本。支持音量控制和通用 ioctl。
