# 音频框架总览 (overview)

XSTAR 音频框架是一套**轻量、模块化、JSON 驱动**的嵌入式音频处理子系统，位于 `xstar/kernel/audio/`。它以经典的 **Source → Filter → Sink** 管道模型为核心，配合插件化的效果引擎与混音器，可支撑从文件播放、麦克风采集、混音、频谱分析到 AFSK 调制解调等多种音频应用。

## 框架定位

| 维度 | 说明 |
|------|------|
| 数据格式 | 内部统一使用 `float` 交错采样 (`audio_frame_t`) |
| 配置方式 | JSON 字符串驱动效果链构建 |
| 扩展模式 | 插件化注册 (`register_audio_effect()`)，自动 initcall 加载 |
| 抽象层级 | `kernel/audio` 提供处理与编排，`driver/audio` 提供硬件 PCM 收发 |
| 代码体量 | 共约 6400 行（含 11 个 effect、11 种 source、6 种 sink、1 个 mixer） |

## 架构总览

```
                ┌─────────────────────────────────────────────┐
                │              kernel/audio                    │
                │                                              │
   ┌────────┐   │   ┌────────┐   ┌─────────┐   ┌────────┐    │   ┌──────────┐
   │ files  │──>│──>│ Source │──>│ Filter  │──>│  Sink  │──>│──>│ playback │
   │ mic    │   │   │ (read) │   │  Chain  │   │ (write)│    │   │  device  │
   │ memory │   │   └────────┘   │         │   └────────┘    │   └──────────┘
   │ tone   │   │        ▲       │ effect1 │        │        │
   │ ...    │   │        │       │ effect2 │        ▼        │
   └────────┘   │   ┌────┴────┐  │  ...    │   ┌─────────┐   │
                │   │  Mixer  │  └─────────┘   │ analyze │   │
                │   │ (mix N) │                │ vad/fft │   │
                │   └─────────┘                └─────────┘   │
                │                                              │
                └─────────────────────────────────────────────┘
                                    │
                                    ▼
                            audio_frame_t  (float interleaved samples)
```

**核心思想**：一切都是带 `filter` 的 `source` 或 `sink`；多个 `source` 经过 `mixer` 合并后，又可作为新的 `source` 继续接入管道。

## 分层模型

| 层级 | 文件 | 职责 |
|------|------|------|
| 1. 数据载体 | `driver/audio/audio.h` | 定义 `audio_frame_t`，提供音量/分贝换算 |
| 2. 效果引擎 | `effect.h` / `effect.c` | 效果插件注册、JSON 解析、滤波器链管理 |
| 3. 源 / 汇 | `source.h` / `sink.h` | 统一的输入输出抽象，内嵌 filter 自动处理 |
| 4. 混音器 | `mixer.h` / `mixer.c` | 多源汇聚、自动归一化、可作为 source 嵌套 |

## 核心数据流

所有组件之间通过 `struct audio_frame_t` 传递数据：

```c
struct audio_frame_t {
    int     rate;     /* 采样率 (Hz)，如 48000 */
    int     channel;  /* 声道数，1=mono, 2=stereo */
    int     frames;   /* 帧数，通常约 5ms 数据 */
    float * samples;  /* 交错样本数据 [L,R,L,R,...] */
};
```

**典型读取流程**：

```
audio_source_read(s)
        │
        ├──> s->read(s)                        ← 由具体 source 实现产出原始 frame
        │
        └──> audio_filter_process(s->filter)   ← 自动应用挂载的效果链
                │
                └──> effect[0].process()
                     effect[1].process()
                     ...
                     return final frame
```

`audio_sink_write()` 反向同理：写入前先经过 sink 自带的 filter 处理。

## 六大设计亮点

### 1. 统一 float 内部格式
所有 source 在数据出口处转为 `float` 交错样本；所有 sink 在硬件写入前再转为目标格式。这样 effect 算法只需面对单一数据类型，大幅简化实现。

### 2. JSON 驱动 + shash 散列
效果链通过 JSON 配置构建，例如：

```json
{
    "resample": { "rate": 48000 },
    "reshape":  { "channel": 2 },
    "volume":   { "factor": 0.7 }
}
```

JSON 字段名解析时使用 `shash()` 计算哈希值，通过 `switch` 跳转避免运行时字符串比较，性能与可读性兼得。详见 [`effect-resample.c:70`](https://github.com/anomalyco/xstar)。

### 3. 插件化注册 + initcall 机制
每个 effect 通过 `register_audio_effect()` 自动注册到全局链表。配合 `core_initcall()` / `pure_initcall()` 多级初始化，无需手动管理依赖顺序：

```c
static struct audio_effect_t volume = {
    .name = "volume", .create = ..., .setup = ..., ...
};
static void effect_volume_init(void)
{
    register_audio_effect(&volume);
}
core_initcall(effect_volume_init);
```

### 4. Filter 复用
同一套 `audio_filter_t` 结构被三处复用：
- `source` 内置 filter — 输入侧预处理
- `sink` 内置 filter — 输出侧后处理
- `mixer` 为每个 source 内部挂载 `resample+reshape` filter 以匹配主格式

### 5. 管道可嵌套组合
`audio_source_alloc_from_mixer()` 可以把整个 mixer 的输出包装成一个新的 `source`，从而构建出多级混音树：

```
[音乐] ─┐
[语音] ─┼─> Mixer A ──┐
[音效] ─┘             ├─> Mixer B ──> Sink
              [提示音] ──┘
```

### 6. 软限幅自动归一化
Mixer 在多源叠加后，若样本超出 `[-1, 1]` 会自动调整全局衰减系数 `mixed`，并以 `1/32` 步长缓慢恢复，实现平滑的动态范围压缩，避免硬削波失真。详见 [`mixer.c:179-191`](https://github.com/anomalyco/xstar)。

## 组件清单

### Source（11 种音频源）

| 名称 | 文件 | 用途 |
|------|------|------|
| `file-wav` | `source-file-wav.c` | WAV 文件解码 |
| `file-qoa` | `source-file-qoa.c` | QOA 文件解码 |
| `capture` | `source-capture.c` | 麦克风采集 |
| `memory` | `source-mem.c` | 内存 PCM 数据 |
| `mixer` | `source-mixer.c` | 把 mixer 包装为 source |
| `tone` | `source-tone.c` | 正弦/方波/三角/锯齿波发生器 |
| `noise` | `source-noise.c` | 白噪声发生器 |
| `afsk` | `source-afsk.c` | AFSK 调制（数据→音频） |
| `custom` | `source-custom.c` | 用户回调函数 |

### Sink（6 种音频汇）

| 名称 | 文件 | 用途 |
|------|------|------|
| `playback` | `sink-playback.c` | 输出到硬件扬声器 |
| `amplitude` | `sink-amplitude.c` | 振幅包络检测 |
| `spectrum` | `sink-spectrum.c` | FFT 频谱分析 |
| `vad` | `sink-vad.c` | 语音活动检测（VAD） |
| `afsk` | `sink-afsk.c` | AFSK 解调（音频→数据） |

### Effect（11 种内建效果）

| 名称 | 用途 |
|------|------|
| `volume` | 音量/分贝调节 |
| `compressor` | 动态范围压缩器（软膝盖、可调 ratio/attack/release/makeup） |
| `limiter` | 峰值限幅（输出保护） |
| `panning` | 多声道矩阵混合 |
| `mono` | 多声道下混为单声道 |
| `duplicate` | 复制音频帧到独立 buffer（隔离后续原地修改） |
| `reshape` | 改变声道数量 |
| `resample` | 改变采样率 |
| `iir` | IIR Biquad 滤波器（低通/高通/带通/陷波等 8 种） |
| `tremolo` | 颤音（振幅调制） |
| `crystalizer` | 高频细节增强 |

详细配置参数与算法参见 [音效插件详解](./effect-plugins)。

## 源码定位表

| 结构体 / 接口 | 位置 |
|---|---|
| `audio_frame_t` | `xstar/driver/audio/audio.h:10` |
| `audio_effect_t` | `xstar/kernel/audio/effect.h:11` |
| `audio_filter_t` | `xstar/kernel/audio/effect.h:27` |
| `audio_source_t` | `xstar/kernel/audio/source.h:12` |
| `audio_sink_t` | `xstar/kernel/audio/sink.h:11` |
| `audio_mixer_t` | `xstar/kernel/audio/mixer.h:10` |
| `audio_filter_process()` | `xstar/kernel/audio/effect.c:219` |
| `audio_mixer_read()` 软限幅 | `xstar/kernel/audio/mixer.c:159` |

## 与驱动层的关系

```
┌─────────────────────────────────────────┐
│        kernel/audio (本框架)              │
│  source / sink / effect / mixer          │
│  ── 处理 / 编排 / 格式转换 ──             │
└────────────┬────────────┬───────────────┘
             │            │
             ▼            ▼
       audio_playback  audio_capture
       (driver/audio)  (driver/audio)
             │            │
             ▼            ▼
         硬件 DAC      硬件 ADC
```

驱动层只提供裸 PCM 收发接口（`audio_playback_write()` / `audio_capture_read()`），所有混音、效果、解码、格式转换均在 kernel 层完成，便于跨平台移植。

## 阅读建议

| 你的需求 | 推荐阅读 |
|---|---|
| 想快速上手写一个播放程序 | [典型使用示例](./usage-examples) → 场景 1 |
| 想理解效果链 JSON 配置 | [音频效果](./audio-effect) → [音效插件详解](./effect-plugins) |
| 想做多源混音 | [音频混音器](./audio-mixer) → [典型使用示例](./usage-examples) 场景 3 |
| 想为自己的格式/算法扩展框架 | [扩展指南](./extending) |
| 想了解每个组件接口 | [音频帧](./audio-frame) / [音频源](./audio-source) / [音频汇](./audio-sink) |
