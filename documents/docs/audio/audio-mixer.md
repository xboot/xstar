# 音频混音器 (audiomixer)

将多个音频源混合为一路输出。

## 结构体

```c
struct audio_mixer_t {
    struct list_head_t list;
    struct mutex_t lock;
    int rate;
    int channel;
    float mixed;
    float factor;
    struct audio_frame_t output;
    float * samples;
    int nsample;
};
```

## API

| 函数 | 说明 |
|------|------|
| `audio_mixer_alloc(rate, channel)` | 分配混音器 |
| `audio_mixer_free(m)` | 释放混音器 |
| `audio_mixer_add(m, source)` | 添加音频源 |
| `audio_mixer_remove(m, source)` | 移除音频源 |
| `audio_mixer_clear(m)` | 清空所有源 |
| `audio_mixer_read(m)` | 读取混音结果 |
| `audio_mixer_get/set_volume(m, vol)` | 获取/设置音量 |

## 说明

混音器内部维护一个 Source 列表，`audio_mixer_read()` 读取所有源的音频帧并混合。混音器本身也可通过 `audio_source_alloc_from_mixer(mixer)` 包装为 Source 使用，实现级联混音。

## 自动格式归一化

调用 `audio_mixer_add()` 加入一个 source 时，混音器会**自动为每个 source 内部挂载 `resample+reshape` filter**，把任意采样率/声道的输入转换为混音器的主格式：

```c
char json[256];
xos_sprintf(json, "{\"resample\":{\"rate\":%d},\"reshape\":{\"channel\":%d}}",
            m->rate, m->channel);
audio_filter_alloc(json, length);
```

因此**无需手动对齐 source 与 mixer 的格式**，可直接混入 16kHz 单声道麦克风与 48kHz 立体声 WAV。

## 软限幅算法

混音器使用动态范围压缩避免硬削波，核心逻辑（见 [`mixer.c:179-191`](https://github.com/anomalyco/xstar)）：

```
For each output sample t = sample * mixed:
    if t > 1.0:
        mixed = 1.0 / t       ← 立即压缩
        t = 1.0
    elif t < -1.0:
        mixed = -1.0 / t
        t = -1.0
    if mixed < 1.0:
        mixed += (1.0 - mixed) / 32.0    ← 缓慢恢复（攻击速度 1/32）
    output = t * factor       ← factor 为用户音量
```

特点：
- **瞬时反应**：发现溢出立刻减小全局系数 `mixed`
- **平滑恢复**：以 `1/32` 步长逐步恢复到 1.0，听觉上无明显泵效应
- **用户音量独立**：`factor` 不参与限幅运算，仅在末端整体缩放

## 级联混音示例

```c
struct audio_mixer_t * sub   = audio_mixer_alloc(48000, 2);
struct audio_mixer_t * main  = audio_mixer_alloc(48000, 2);

audio_mixer_add(sub, src_voice);
audio_mixer_add(sub, src_effect);

/* 把 sub 包装成 source 喂给 main */
struct audio_source_t * sub_as_src = audio_source_alloc_from_mixer(sub);
audio_mixer_add(main, sub_as_src);
audio_mixer_add(main, src_bgm);

/* 从 main 读取最终混音 */
struct audio_frame_t * af = audio_mixer_read(main);
```

完整使用示例参见 [典型使用示例](./usage-examples) 场景 3。
