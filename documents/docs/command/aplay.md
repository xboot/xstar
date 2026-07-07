# aplay

音频播放命令，支持可配置的输入源、输出目标和效果链。

## 用法

```
aplay -i=<source> -o=<sink> [-ie=<json>] [-oe=<json>] [-l]
```

## 参数

| 参数 | 说明 |
|------|------|
| `-i=<source>` | 输入源（必需） |
| `-o=<sink>` | 输出目标（必需） |
| `-ie=<json>` | 输入侧效果链 JSON |
| `-oe=<json>` | 输出侧效果链 JSON |
| `-l` | 循环播放，源结束后自动从头重新播放 |

## 输入源

| 类型 | 格式 | 说明 |
|------|------|------|
| file | `file:<path>` | 播放 XFS 中的音频文件（WAV/QOA） |
| capture | `capture:<device>,<rate>,<ch>` | 从麦克风采集音频 |
| tone | `tone:<waveform>,<rate>,<ch>,<freq>[,<ms>]` | 波形发生器 |
| noise | `noise:<rate>,<ch>` | 白噪声发生器 |

- `waveform`：`sine`、`square`、`triangle`、`sawtooth`
- `ms`：持续时间（毫秒），省略或为 0 表示无限循环
- `rate`：采样率（如 48000）
- `ch`：声道数（1=单声道，2=立体声）

## 输出目标

| 类型 | 格式 | 说明 |
|------|------|------|
| playback | `playback:<device>,<rate>,<ch>` | 播放到硬件音频设备 |
| amplitude | `amplitude:<period_ms>` | 振幅包络分析 |
| spectrum | `spectrum:<period_ms>` | FFT 频谱分析 |
| vad | `vad:<start_hz>,<end_hz>` | 语音活动检测 |
| afsk | `afsk:<mark_hz>,<space_hz>,<rate>,<bitrate>` | AFSK 解调 |

## 效果链

`-ie` 和 `-oe` 接受 JSON 对象，键名为效果名，键值为效果参数。多个效果按 JSON 键的书写顺序依次处理。

可用效果：

| 效果名 | 参数 | 说明 |
|--------|------|------|
| volume | `{"decibel": -6.0}` 或 `{"factor": 0.5}` | 音量调节 |
| iir | `{"type": "lowpass", "frequency": 2000}` | IIR 滤波器（lowpass/highpass/bandpass/lowshelf/highshelf/peaking/notch/allpass） |
| compressor | `{"threshold": -20, "ratio": 4}` | 动态范围压缩 |
| limiter | `{"threshold": -1, "release": 50}` | 峰值限制 |
| resample | `{"rate": 48000}` | 采样率转换 |
| reshape | `{"channel": 2}` | 声道数转换 |
| mono | `{}` | 多声道混合为单声道 |
| panning | `{"weight": [1.0, 0.0, 0.0, 1.0]}` | 声道重映射 |
| duplicate | `{}` | 缓冲复制（隔离原地修改） |
| tremolo | `{"frequency": 5.0, "depth": 0.5}` | 颤音效果 |
| crystalizer | `{"intensity": 2.0}` | 声音增强 |

## 示例

```bash
# 播放音频文件（循环）
aplay -i=file:music.wav -o=playback:playback-linux.0,48000,2 -l

# 正弦波 440Hz 测试音
aplay -i=tone:sine,48000,2,440 -o=playback:playback-linux.0,48000,2

# 白噪声测试
aplay -i=noise:48000,2 -o=playback:playback-linux.0,48000,2

# 麦克风直通到喇叭
aplay -i=capture:capture-linux.0,48000,2 -o=playback:playback-linux.0,48000,2

# 播放文件 + 输入侧效果链
aplay -i=file:music.wav -o=playback:playback-linux.0,48000,2 -l \
  -ie='{"volume":{"decibel":-6},"iir":{"type":"lowpass","frequency":2000}}'

# 播放文件 + 输出侧限幅
aplay -i=file:music.wav -o=playback:playback-linux.0,48000,2 -l \
  -oe='{"limiter":{"threshold":-1}}'

# 麦克风语音活动检测
aplay -i=capture:capture-linux.0,16000,1 -o=vad:300,800

# 麦克风频谱分析
aplay -i=capture:capture-linux.0,48000,2 -o=spectrum:50
```
