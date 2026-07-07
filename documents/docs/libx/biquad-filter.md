# 双二阶滤波器 (biquad)

双二阶（Biquadratic）IIR 滤波器，支持 8 种常见滤波类型，用于音频和信号处理中的频率响应整形。

## 原理

双二阶滤波器是二阶 IIR 滤波器的标准实现，其传递函数为：

```
H(z) = (b0 + b1·z⁻¹ + b2·z⁻²) / (1 + a1·z⁻¹ + a2·z⁻²)
```

对应的差分方程：

```
y[n] = b0·x[n] + b1·x[n-1] + b2·x[n-2] - a1·y[n-1] - a2·y[n-2]
```

系数根据滤波类型由采样率、截止频率、品质因数 Q 和增益计算得出，参考 [BiQuadDesigner](https://arachnoid.com/BiQuadDesigner)。

## 滤波类型

| 枚举值 | 类型 | 说明 |
|--------|------|------|
| `BIQUAD_FILTER_TYPE_LOWPASS` | 低通 | 衰减高频，保留低频 |
| `BIQUAD_FILTER_TYPE_HIGHPASS` | 高通 | 衰减低频，保留高频 |
| `BIQUAD_FILTER_TYPE_BANDPASS` | 带通 | 仅保留中心频率附近频段 |
| `BIQUAD_FILTER_TYPE_LOWSHELF` | 低频搁架 | 对低频进行增益或衰减 |
| `BIQUAD_FILTER_TYPE_HIGHSHELF` | 高频搁架 | 对高频进行增益或衰减 |
| `BIQUAD_FILTER_TYPE_PEAKING` | 峰值 | 对指定频率进行增益或衰减 |
| `BIQUAD_FILTER_TYPE_NOTCH` | 陷波 | 衰减指定频率（窄带） |
| `BIQUAD_FILTER_TYPE_ALLPASS` | 全通 | 仅改变相位，不改变幅度 |

## 数据结构

```c
struct biquad_filter_t {
    float b0, b1, b2;   /* 分子系数 */
    float a1, a2;       /* 分母系数（a0 已归一化为 1） */
    float xn1, xn2;     /* 输入历史：x[n-1], x[n-2] */
    float yn1, yn2;     /* 输出历史：y[n-1], y[n-2] */
};
```

## API

```c
struct biquad_filter_t * biquad_alloc(enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain);
```

分配并初始化滤波器。`samplerate` 为采样率（Hz），`frequency` 为截止/中心频率（Hz），`quality` 为品质因数 Q，`gain` 为增益（dB，仅对搁架和峰值类型有效）。失败返回 `NULL`。

```c
void biquad_free(struct biquad_filter_t * filter);
```

释放滤波器。

```c
void biquad_init(struct biquad_filter_t * filter, enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain);
```

初始化已有滤波器结构的系数，不分配内存。调用后需手动调用 `biquad_clear` 清除历史状态。

```c
void biquad_clear(struct biquad_filter_t * filter);
```

清除滤波器的历史输入输出状态（xn1、xn2、yn1、yn2 置零），不改变系数。

```c
void biquad_process(struct biquad_filter_t * filter, float * input, float * output, int len);
```

对输入缓冲区 `input` 中的 `len` 个采样点执行滤波，结果写入 `output`。支持块处理，历史状态在调用间自动保持。

## 使用示例

```c
struct biquad_filter_t * f = biquad_alloc(BIQUAD_FILTER_TYPE_LOWPASS, 44100.0f, 1000.0f, 0.707f, 0.0f);

float in[256], out[256];
while(read_audio(in, 256))
{
    biquad_process(f, in, out, 256);
    write_audio(out, 256);
}

biquad_free(f);
```
