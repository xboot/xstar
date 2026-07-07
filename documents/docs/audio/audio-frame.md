# 音频帧 (audioframe)

音频数据的基本传递单位。

## 结构体

```c
struct audio_frame_t {
    int rate;         /* 采样率 (Hz) */
    int channel;      /* 声道数 */
    int frames;       /* 帧数 (~5ms 音频数据) */
    float * samples;  /* 交织样本数据 */
};
```

## 辅助函数

| 函数 | 说明 |
|------|------|
| `audio_frame_is_valid(af)` | 检查音频帧是否有效 |
| `audio_frame_total_time(af)` | 获取音频帧总时长 (ms) |
| `volume_to_factor(volume)` | 音量 [0,1000] 转系数 [-50db,0db] |
| `factor_to_volume(factor)` | 系数转音量 [0,1000] |

## 说明

音频帧是音频处理链路中传递数据的基本单元。`samples` 为 float 类型的交织数据（如立体声为 L/R/L/R...）。`frames` 通常对应约 5ms 的音频数据。
