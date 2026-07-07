# 典型使用示例 (usage-examples)

本文通过 4 个完整可运行的场景，演示 XSTAR 音频框架的常用模式。每个示例包含 **目标 → 数据流图 → 代码 → 关键说明** 四部分。

## 场景 1：播放 WAV 文件 + 音量调节

**目标**：从 XFS 文件系统读取 `music.wav`，应用 70% 音量后送扬声器播放。

**数据流**：
```
   music.wav (xfs)
        │
        ▼
   ┌──────────┐   filter:    ┌─────────────┐
   │ Source   │──{volume}──> │ Sink        │──> 扬声器
   │ file-wav │              │ playback    │
   └──────────┘              └─────────────┘
```

**代码**：
```c
#include <xstar.h>
#include <kernel/audio/source.h>
#include <kernel/audio/sink.h>

void play_music(struct xfs_context_t * ctx)
{
    struct audio_source_t * src;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. 创建 WAV 源 */
    src = audio_source_alloc_from_xfs(ctx, "/music.wav");
    if(!src)
        return;

    /* 2. 应用 70% 音量 filter */
    audio_source_filter_apply(src,
        "{\"volume\":{\"factor\":0.7}}", -1);

    /* 3. 创建播放 sink (48k 立体声) */
    snk = audio_sink_alloc_from_playback(NULL, 48000, 2);
    if(!snk)
    {
        audio_source_free(src);
        return;
    }

    /* 4. 主循环：边读边写 */
    while((af = audio_source_read(src)) && audio_frame_is_valid(af))
        audio_sink_write(snk, af);

    /* 5. 清理 */
    audio_sink_free(snk);
    audio_source_free(src);
}
```

**关键说明**：
- `audio_source_alloc_from_xfs()` 根据扩展名自动选择 wav/qoa 解码器
- `audio_source_filter_apply()` 的 JSON `len` 参数为 `-1` 时按 `\0` 计算
- `audio_sink_alloc_from_playback()` 内部自动挂载 `resample+reshape` filter，源音频与硬件参数不一致也能自动适配
- 主循环退出条件：`af == NULL` 或 `audio_frame_is_valid(af) == 0`

---

## 场景 2：麦克风录音 + VAD 语音活动检测

**目标**：从麦克风采集音频，检测是否有人在说话，触发回调。

**数据流**：
```
   麦克风
        │
        ▼
   ┌──────────┐              ┌─────────┐
   │ Source   │─────────────>│ Sink    │──> 回调:语音开始/结束
   │ capture  │              │ vad     │
   └──────────┘              └─────────┘
```

**代码**：
```c
void voice_detect_loop(void)
{
    struct audio_source_t * src;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. 麦克风源：16kHz 单声道 */
    src = audio_source_alloc_from_capture(NULL, 16000, 1);

    /* 2. VAD sink：start=300ms (持续 300ms 高能量判定开始)
     *               end=800ms   (持续 800ms 低能量判定结束) */
    snk = audio_sink_alloc_vad(300, 800);

    while(running)
    {
        af = audio_source_read(src);
        if(audio_frame_is_valid(af))
            audio_sink_write(snk, af);
    }

    audio_sink_free(snk);
    audio_source_free(src);
}
```

**关键说明**：
- `sink-vad` 内部实现完整 VAD 算法（能量阈值 + 滑动窗口）
- 通过 `audio_sink_ioctl(snk, "vad-get-state", ...)` 查询当前语音活动状态
- 录音速率与 sink 期望不一致时，可在 source 上挂 `resample` filter

---

## 场景 3：多源混音 + 输出到扬声器

**目标**：背景音乐 + 提示音 + 麦克风同时混音输出。

**数据流**：
```
   bgm.wav  ──> Source A ──┐
                            │
   beep.wav ──> Source B ──┼──> Mixer (48k stereo) ──> Sink playback
                            │            │
   麦克风    ──> Source C ──┘     auto: resample+reshape
                                   per source
```

**代码**：
```c
void mix_three_sources(struct xfs_context_t * ctx)
{
    struct audio_mixer_t  * mixer;
    struct audio_source_t * src_a, * src_b, * src_c;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. 创建 48k 立体声混音器 */
    mixer = audio_mixer_alloc(48000, 2);

    /* 2. 创建 3 个 source */
    src_a = audio_source_alloc_from_xfs(ctx, "/bgm.wav");
    src_b = audio_source_alloc_from_xfs(ctx, "/beep.wav");
    src_c = audio_source_alloc_from_capture(NULL, 16000, 1);

    /* 3. 加入混音器（mixer 自动为每个 source 加 resample+reshape） */
    audio_mixer_add(mixer, src_a);
    audio_mixer_add(mixer, src_b);
    audio_mixer_add(mixer, src_c);

    /* 4. 设定混音器总音量 70% */
    audio_mixer_set_volume(mixer, 700);

    /* 5. 创建输出 sink */
    snk = audio_sink_alloc_from_playback(NULL, 48000, 2);

    /* 6. 主循环 */
    while(running)
    {
        af = audio_mixer_read(mixer);
        if(audio_frame_is_valid(af))
            audio_sink_write(snk, af);
    }

    /* 7. 清理 */
    audio_sink_free(snk);
    audio_mixer_free(mixer);        /* 自动清空内部列表 */
    audio_source_free(src_a);
    audio_source_free(src_b);
    audio_source_free(src_c);
}
```

**关键说明**：
- `audio_mixer_add()` 会自动给每个 source 包装 `resample+reshape` 滤波链，无需手动统一格式
- mixer 自带**软限幅算法**，多源叠加溢出时自动平滑衰减
- mixer 也可包装为 source：`audio_source_alloc_from_mixer(mixer)`，实现多级混音树

---

## 场景 4：音调发生器 + FFT 频谱分析

**目标**：生成 1kHz 正弦波，实时计算其频谱用于显示。

**数据流**：
```
   ┌────────────┐    ┌────────────┐
   │ Source     │───>│ Sink       │──> 周期性产出 FFT 频谱数组
   │ tone(sine) │    │ spectrum   │
   └────────────┘    └────────────┘
```

**代码**：
```c
void show_spectrum(void)
{
    struct audio_source_t * src;
    struct audio_sink_t   * snk;
    struct audio_frame_t  * af;

    /* 1. 1kHz 正弦波，48k 单声道，时长 5 秒 */
    src = audio_source_alloc_tone("sine", 48000, 1, 1000, 5000);

    /* 2. 频谱分析 sink，周期 50ms 输出一次频谱 */
    snk = audio_sink_alloc_spectrum(50);

    /* 3. 馈送数据 */
    while((af = audio_source_read(src)) && audio_frame_is_valid(af))
    {
        audio_sink_write(snk, af);

        /* 4. 通过 ioctl 取得最新频谱数据 */
        float * fft_bins = NULL;
        int     nbins    = 0;
        struct {
            float ** bins;
            int    * n;
        } arg = { &fft_bins, &nbins };
        if(audio_sink_ioctl(snk, "spectrum-get-bins", &arg) >= 0)
        {
            /* 把 fft_bins[0..nbins-1] 渲染到屏幕... */
        }
    }

    audio_sink_free(snk);
    audio_source_free(src);
}
```

**关键说明**：
- `tone` 支持 4 种波形：`"sine"` / `"square"` / `"triangle"` / `"sawtooth"`
- 时长参数为毫秒，传 0 表示循环播放
- `sink-spectrum` 内部使用 libx 的 FFT 模块；`period` 决定输出频率（ms）
- `ioctl` 命令字符串规范见 [音频汇 ioctl 命令](./audio-sink#ioctl)

---

## 模式总结

| 场景 | 关键组合 |
|------|---------|
| 单源播放 | `source-file` + (optional filter) + `sink-playback` |
| 录音分析 | `source-capture` + `sink-vad` / `sink-spectrum` / `sink-amplitude` |
| 多源混音 | `source × N` + `mixer` + `sink-playback` |
| 信号合成 | `source-tone` / `source-noise` / `source-custom` + `sink-*` |
| 数据传输 | `source-afsk` ↔ `sink-afsk` （音频调制解调） |
| 内存音效 | `source-mem` + `filter` + `sink-playback` |

更多扩展场景请参考 [扩展指南](./extending)。
