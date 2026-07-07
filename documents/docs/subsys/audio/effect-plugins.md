# 音效插件详解 (effect-plugins)

XSTAR 音频框架内建 **11 个音效插件**，全部位于 `xstar/kernel/audio/effect-*.c`。本文按"格式转换 → 信号处理 → 特效"三大类逐一介绍其用途、JSON 配置与算法要点。

## 速查总表

| 类别 | 名称 | JSON 关键参数 | 默认值 | 是否带状态 |
|------|------|--------------|--------|-----------|
| 格式 | `resample` | `rate` (int) | 48000 | 是（fraction 累加器） |
| 格式 | `reshape` | `channel` (int) | 1 | 否 |
| 格式 | `mono` | — | — | 否 |
| 格式 | `panning` | `weight` (array) | 单位矩阵 | 否 |
| 工具 | `duplicate` | — | — | 否（仅 memcpy） |
| 信号 | `volume` | `factor` 或 `decibel` (double) | 1.0 / 0dB | 否 |
| 信号 | `compressor` | `threshold` (dBFS) / `ratio` / `attack` (ms) / `release` (ms) / `knee` (dB) / `makeup` (dB) | -20 / 4 / 5 / 100 / 6 / 0 | 是（包络） |
| 信号 | `limiter` | `threshold` (dBFS) / `release` (ms) | -1.0 / 50 | 是（瞬时增益） |
| 信号 | `iir` | `type` / `frequency` / `quality` / `gain` | lowpass/1000/0.707/6.0 | 是 |
| 特效 | `tremolo` | `frequency` / `depth` (double) | 5.0 / 0.5 | 是（相位） |
| 特效 | `crystalizer` | `intensity` (double) / `clip` (bool) | 2.0 / true | 是（前一样本） |

---

## 一、格式转换 / 帧处理类

### 1. resample — 采样率转换

**用途**：将输入采样率转换为目标采样率。常用于把不同来源的音频统一到硬件支持的速率。

**JSON 配置**：
```json
{
    "resample": {
        "rate": 48000
    }
}
```

**算法要点**：使用基于 `fraction` 累加器的简单线性间隔重采样（最邻近插值）。每输入一帧 `fraction` 递增 1，超过 1 时输出一帧并扣除步长 `step = input_rate / target_rate`。

**源码位置**：`xstar/kernel/audio/effect-resample.c:104`

> 💡 该实现追求轻量与零延迟，**没有抗混叠滤波器**。若需要更高保真度，可在 resample 之前串接 `iir` 低通滤波。

---

### 2. reshape — 声道数量变换

**用途**：改变音频帧的声道数。多声道转少声道时取前 N 个；少声道转多声道时把剩余声道**复制最后一个已写入声道**的样本值。

**JSON 配置**：
```json
{
    "reshape": {
        "channel": 2
    }
}
```

**源码位置**：`xstar/kernel/audio/effect-reshape.c`

---

### 3. mono — 下混为单声道

**用途**：将所有声道**平均叠加**输出单声道，比 `reshape` 更适合保留全部声道信息。

**JSON 配置**：
```json
{
    "mono": {}
}
```

**算法**：`out[n] = (in[n*ch + 0] + in[n*ch + 1] + ... + in[n*ch + ch-1]) / ch`

**源码位置**：`xstar/kernel/audio/effect-mono.c`

---

### 4. duplicate — 复制音频帧（隔离原地修改）

**用途**：把输入 `audio_frame_t` 的样本数据 `memcpy` 到本节点的私有 buffer，再以新帧返回。**格式与样本值完全不变**（rate/channel/frames 全部相同）。

**JSON 配置**：
```json
{
    "duplicate": {}
}
```

**为什么需要它**：framework 中的各个 effect 对输入帧的处理方式并不统一——
- 有些 effect **原地修改** input->samples（如 `volume`、`tremolo`、`crystalizer`）
- 有些 effect **不修改** input，而是把结果写入自己的私有 buffer 后返回（如 `resample`、`reshape`、`mono`、`duplicate` 自己）

当上游希望**保留原始帧数据**给其它路径使用时，可在链路前部插入 `duplicate`，使后续会原地修改的 effect 改写的是副本而不是原始数据。

**示例场景**：同一份 source 既要送扬声器（经 volume 衰减），又要送频谱分析（需要原始电平）——
```
source ──> duplicate ──> volume ──> sink-playback
                 │
                 └─────> 另一处用 audio_filter_process 走频谱通路（不受 volume 影响）
```

**源码位置**：`xstar/kernel/audio/effect-duplicate.c:72`

---

### 5. panning — 声道矩阵重映射

**用途**：用 N×N 权重矩阵对声道进行线性组合，可实现声像调整、立体声转单声道、多声道 downmix 等。

**JSON 配置**（2×2 立体声直通示例）：
```json
{
    "panning": {
        "weight": [
            1.0, 0.0,
            0.0, 1.0
        ]
    }
}
```

立体声转单声道（两声道各取一半）：
```json
{
    "panning": {
        "weight": [ 0.5, 0.5 ]
    }
}
```

立体声左右声道对换（L↔R）：
```json
{
    "panning": {
        "weight": [
            0.0, 1.0,
            1.0, 0.0
        ]
    }
}
```

**算法**：`out[v] = Σ weight[u][v] * in[u]`，最多支持 32 声道。

**源码位置**：`xstar/kernel/audio/effect-panning.c`

---

## 二、信号处理类

### 6. volume — 音量/分贝调节

**用途**：按系数或分贝值整体缩放幅度。

**JSON 配置**（两种二选一）：
```json
{
    "volume": { "factor": 0.5 }
}
```
```json
{
    "volume": { "decibel": -6.0 }
}
```

**换算**：`factor = 10^(decibel/20)`，即 `-6dB ≈ 0.501`。

**源码位置**：`xstar/kernel/audio/effect-volume.c:89`

---

### 7. compressor — 动态范围压缩器

**用途**：当输入电平超过阈值时，按设定的压缩比 N:1 平滑地减小增益，从而**压缩动态范围**：使响的部分变轻、轻的部分相对变响（再配合 `makeup` 提升整体响度）。与 `limiter` 的"硬天花板瞬时压"不同，compressor 提供可控的 `ratio`、`attack`、`release`、`knee` 与 `makeup`，听感更自然，是播放/语音/录音链中的基础动态工具。

**JSON 配置**：
```json
{
    "compressor": {
        "threshold": -20.0,
        "ratio": 4.0,
        "attack": 5.0,
        "release": 100.0,
        "knee": 6.0,
        "makeup": 0.0
    }
}
```

**参数说明**：

| 参数 | 含义 | 单位 | 范围 | 默认值 |
|---|---|:---:|:---:|:---:|
| `threshold` | 软膝盖中心，超过此电平开始压缩 | dBFS | [-60.0, 0.0] | -20.0 |
| `ratio` | 压缩比 N:1（输入每增加 N dB，输出仅增加 1 dB） | — | [1.0, 100.0] | 4.0 |
| `attack` | 起振时间（电平上升时增益跟随速度） | ms | [0.1, 1000.0] | 5.0 |
| `release` | 释放时间（电平下降时增益恢复速度） | ms | [1.0, 5000.0] | 100.0 |
| `knee` | 软拐点宽度（threshold 周围的过渡区） | dB | [0.0, 24.0] | 6.0 |
| `makeup` | 补偿增益（在压缩后整体放大） | dB | [-24.0, 24.0] | 0.0 |

**算法**（log 域，feed-forward）：
```
每帧处理：
  1. 取该帧所有声道的最大绝对值 peak
  2. 转 dB： in_db = 20 * log10(peak)
  3. 计算增益衰减 gr_db（带软膝盖）：
       over = in_db - threshold
       slope = 1 - 1/ratio
       若 over <= -knee/2 : gr_db = 0
       若 over >=  knee/2 : gr_db = slope * over
       其它（软膝盖区）   : gr_db = slope * (over + knee/2)^2 / (2 * knee)
  4. 包络跟随（不对称单极平滑）：
       coef = (gr_db > env_db) ? attack_coef : release_coef
       env_db += (gr_db - env_db) * coef
  5. 总增益： gain = 10^((-env_db + makeup)/20)
  6. 所有声道乘以 gain（共享增益，保持立体声像）
```

attack / release 系数计算与 limiter 一致：
- `attack_coef  = 1 - exp(-1000 / (attack  * rate))`
- `release_coef = 1 - exp(-1000 / (release * rate))`

**ratio 取值建议**：

| ratio | 听感 | 典型用途 |
|:---:|---|---|
| 1.5 ~ 2.0 | 极轻微 | 母带轻度黏合 |
| 2.0 ~ 4.0 | 自然 | **人声、整体响度控制（推荐起点）** |
| 4.0 ~ 8.0 | 明显 | 鼓组、贝斯、需要稳定电平的场景 |
| 8.0 ~ 20.0 | 强烈 | 接近限幅器，用于硬控制 |
| > 20.0 | 类限幅 | 近似 limiter 行为 |

**compressor vs limiter**：

| 维度 | compressor | limiter |
|---|---|---|
| 触发方式 | 软膝盖平滑过渡 | 瞬时硬触发 |
| 压缩比 | 可调（1:1 ~ 100:1） | 固定 ∞:1 |
| attack | 可调 (ms 级) | 0（零起振） |
| 用途 | 控制动态、提升响度 | 防止削波兜底 |
| 串联位置 | 信号链前段/中段 | 信号链最末端 |

> 💡 **典型组合**：`compressor (ratio 3:1, threshold -20, makeup +6dB)` → … → `limiter (-1 dBFS)`，先做动态控制，最后由 limiter 兜底防削波。

**源码位置**：`xstar/kernel/audio/effect-compressor.c`

---

### 8. limiter — 峰值限幅器

**用途**：当输入样本峰值超过阈值时瞬时压缩，并在释放时间内平滑恢复到单位增益。零起振时间（instant attack），确保不会有任何样本超过设定天花板。常用于输出保护与整体响度控制。

**JSON 配置**：
```json
{
    "limiter": {
        "threshold": -1.0,
        "release": 50.0
    }
}
```

**参数说明**：
- `threshold`：限幅天花板，单位 **dBFS**（dB Full Scale，相对于满刻度 ±1.0 的分贝值），范围 [-60.0, 0.0]，默认 -1.0 dBFS。`0 dBFS` 对应线性幅度 1.0（即浮点样本的满刻度），`-1.0 dBFS` 对应约 0.891，超过此值的峰值会被瞬时压下。换算公式：`ceiling = 10^(threshold/20)`。
- `release`：释放时间 (ms)，范围 [1.0, 5000.0]，默认 50 ms。控制限幅触发后增益按一阶低通向 1.0 回复的速度（约 release ms 内恢复 63.2%）。

**dBFS 与线性幅度对照表**：

| threshold (dBFS) | ceiling (线性幅度) | 含义 |
|:---:|:---:|---|
| 0 | 1.000 | 满刻度，等于不限幅 |
| -1 | 0.891 | **默认值**，留 1 dB 余量 |
| -3 | 0.708 | 约一半功率 |
| -6 | 0.501 | 幅度减半 |
| -20 | 0.100 | 幅度为 1/10 |
| -40 | 0.010 | 幅度为 1/100 |
| -60 | 0.001 | 幅度为 1/1000，几乎静音 |

**threshold 取值建议**：

| 使用场景 | 推荐 threshold | 说明 |
|---|:---:|---|
| 母带处理 / 最终输出保护 | -0.3 ~ -1.0 dBFS | 留余量防止 D/A 转换后的 inter-sample peak 削波 |
| 一般播放保护（默认） | -1.0 dBFS | 兼顾响度与安全，适合大多数场景 |
| 需要明显压制响度 | -3 ~ -6 dBFS | 会听到明显的动态压缩感 |
| 强力限幅 | < -6 dBFS | 会明显改变音色，慎用 |

**算法**：
```
每帧处理：
  1. 找出帧内所有声道中的最大绝对值 peak
  2. 若 peak * gain > ceiling：目标增益 = ceiling / peak（瞬时压缩）
  3. 否则：gain 按一阶低通向 1.0 逼近（smooth recovery）
  4. 用 gain 缩放所有声道样本（共享增益，保持立体声像）
```

释放系数计算：`coef = 1 - exp(-1000 / (release * rate))`，其中 `rate` 为当前帧的采样率。

**源码位置**：`xstar/kernel/audio/effect-limiter.c`

---

### 9. iir — IIR Biquad 滤波器（最强大）

**用途**：二阶 IIR Biquad 滤波器，支持 8 种类型。可用于低通、高通、带通、陷波、均衡器等场景。

**JSON 配置**：
```json
{
    "iir": {
        "type": "lowpass",
        "frequency": 1000.0,
        "quality": 0.707,
        "gain": 6.0
    }
}
```

**type 取值**：

| 类型 | 用途 |
|------|------|
| `"lowpass"` | 低通滤波（默认） |
| `"highpass"` | 高通滤波 |
| `"bandpass"` | 带通滤波 |
| `"lowshelf"` | 低频架（均衡器低音） |
| `"highshelf"` | 高频架（均衡器高音） |
| `"peaking"` | 峰值（均衡器中音） |
| `"notch"` | 陷波（消除特定频率） |
| `"allpass"` | 全通（仅改变相位） |

**参数说明**：
- `frequency`：截止/中心频率 (Hz)
- `quality`：品质因子 Q（0.707 为巴特沃斯响应）
- `gain`：增益 dB（仅 shelf / peaking 类型使用）

**算法**：标准 Biquad 差分方程 `y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]`，系数按 Robert Bristow-Johnson 设计公式计算。每声道独立维护历史状态 `xn1/xn2/yn1/yn2`。

**在线设计工具**：[https://arachnoid.com/BiQuadDesigner](https://arachnoid.com/BiQuadDesigner)

**源码位置**：`xstar/kernel/audio/effect-iir.c:101`

---

## 三、特效类

### 10. tremolo — 颤音

**用途**：周期性调制音量幅度，产生颤动效果（如老电吉他、复古效果）。

**JSON 配置**：
```json
{
    "tremolo": {
        "frequency": 5.0,
        "depth": 0.5
    }
}
```

**参数说明**：
- `frequency`：调制频率 (Hz)，0.1 ~ 20000，常用 4 ~ 8 Hz
- `depth`：调制深度 0.0 ~ 1.0，0 = 无效果，1 = 完全静音深度

**算法**：内部预生成正弦查表，对每个样本乘以 `1 - depth + depth * sin(2π*f*t)`。

**源码位置**：`xstar/kernel/audio/effect-tremolo.c`

---

### 11. crystalizer — 高频细节增强

**用途**：通过对样本变化率乘以系数实现高频提升，让音频"更清晰"。本质上是一种简单高通增强。

**JSON 配置**：
```json
{
    "crystalizer": {
        "intensity": 2.0,
        "clip": true
    }
}
```

**参数说明**：
- `intensity`：增强强度 -10.0 ~ 10.0
- `clip`：是否硬限幅到 `[-1, 1]`

**算法**：`out[n] = in[n] + intensity * (in[n] - in[n-1])`

**源码位置**：`xstar/kernel/audio/effect-crystalizer.c`

---

## 链式组合示例

多个 effect 可串联成滤波链，按 JSON 顺序依次处理：

```json
{
    "resample":    { "rate": 48000 },
    "reshape":     { "channel": 2 },
    "iir":         { "type": "lowpass", "frequency": 8000.0 },
    "compressor":  { "threshold": -20.0, "ratio": 3.0, "makeup": 6.0 },
    "volume":      { "decibel": -3.0 },
    "crystalizer": { "intensity": 1.5 },
    "limiter":     { "threshold": -1.0, "release": 50.0 }
}
```

> ⚠️ 顺序很重要：通常**先做格式归一化**（resample/reshape），**再做信号处理**（iir/compressor/volume），**然后做特效**，**最后用 limiter 兜底**防止削波。

> 📌 **未知 effect 的处理**：链中出现**未注册或拼写错误**的 effect 名（例如 `"limitter"`、`"echo"`）会被 `audio_filter_alloc()` **静默跳过**，不会报错也不会中断后续 effect 的解析；已识别的 effect 仍按 JSON 中的原始顺序串接。例如：
>
> ```json
> {
>     "volume":   { "decibel": -3.0 },
>     "limitter": { "threshold": -1.0 },   // 拼写错误，被忽略
>     "echo":     { "delay": 100 },        // 未实现，被忽略
>     "limiter":  { "threshold": -1.0 }
> }
> ```
>
> 实际生效的链为 `volume → limiter`，共 2 个节点。若整个 JSON 对象中**没有任何**已注册的 effect 名，`audio_filter_alloc()` 返回 `NULL`。该行为意味着拼写错误不会引发崩溃，但也不会有任何提示，请在调试期对照 [速查总表](#速查总表) 仔细核对 effect 名称。

## 如何选择 effect

| 你的需求 | 推荐 effect |
|---------|------------|
| 多源混音前对齐采样率/声道 | `resample` + `reshape` |
| 整体音量控制 | `volume` |
| 立体声转单声道 | `mono` 或 `panning` |
| 单声道扩成立体声 | `reshape` (channel=2) 或 `panning` |
| 隔离后续 effect 的原地修改 | `duplicate` |
| 低频/高频增强 | `iir` (lowshelf/highshelf) |
| 消除特定频率噪声 | `iir` (notch) |
| 复古颤音效果 | `tremolo` |
| 提升音频清晰度 | `crystalizer` |
| 控制动态范围 / 让响度更稳定 / 提升整体响度 | `compressor` |
| 防止输出削波 / 整体响度控制 | `limiter` |

## 注册机制

所有 effect 通过 `core_initcall()` 自动注册，源码统一遵循以下模板（详见 [扩展指南](./extending)）：

```c
static struct audio_effect_t my_effect = {
    .name    = "myeffect",
    .create  = my_create,
    .setup   = my_setup,
    .prepare = my_prepare,
    .process = my_process,
    .destroy = my_destroy,
};
static void my_effect_init(void) { register_audio_effect(&my_effect); }
core_initcall(my_effect_init);
```
