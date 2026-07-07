# 音频捕获 (audiocapture)

音频捕获（麦克风输入）。

## 设备类型

`DEVICE_TYPE_AUDIOCAPTURE`

## 结构体

```c
struct audio_capture_t {
    char * name;
    int (*start)(struct audio_capture_t * capture, int rate, int channel);
    int (*read)(struct audio_capture_t * capture, float * samples, int nsample);
    int (*stop)(struct audio_capture_t * capture);
    int (*ioctl)(struct audio_capture_t * capture, const char * cmd, void * arg);
    void * priv;
};
```

## 关键 API

| 函数 | 说明 |
|------|------|
| `search_audio_capture(name)` | 按名称查找音频捕获设备 |
| `register_audio_capture(capture, drv)` | 注册音频捕获设备 |
| `unregister_audio_capture(capture)` | 注销音频捕获设备 |
| `audio_capture_start/stop(capture, ...)` | 启动/停止捕获 |
| `audio_capture_read(capture, samples, nsample)` | 读取音频样本 |
| `audio_capture_get/set_volume(capture)` | 获取/设置音量 |

## 说明

音频捕获（麦克风输入）接口。以指定采样率和通道数捕获音频样本，返回 float 样本。支持音量控制和通用 ioctl。
