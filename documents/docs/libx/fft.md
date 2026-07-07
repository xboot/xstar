# 快速傅里叶变换 (fft)

快速傅里叶变换（FFT），对 2 的幂次长度数据执行离散傅里叶正变换与逆变换，使用预计算查找表和位反转置换。

## API

```c
struct fft_t * fft_new(int size_log2);
void fft_free(struct fft_t * fft);
void fft_process_forward_complex(struct fft_t * fft, struct complex_t * out, struct complex_t * in);
void fft_process_forward(struct fft_t * fft, struct complex_t * out, float * in);
void fft_process_inverse(struct fft_t * fft, float * out, struct complex_t * in);
```

- `fft_new` — 创建 FFT 上下文，长度 2^size_log2
- `fft_free` — 销毁
- `fft_process_forward_complex` — 正变换，复数输入→复数输出
- `fft_process_forward` — 正变换，实数输入→复数输出
- `fft_process_inverse` — 逆变换，复数输入→实数输出

## 使用示例

```c
struct fft_t * fft = fft_new(10);
struct complex_t out[1024];
float in[1024];

for (int i = 0; i < 1024; i++)
	in[i] = sinf(2.0f * M_PI * 50.0f * i / 1024.0f);

fft_process_forward(fft, out, in);

float amp[512];
for (int i = 0; i < 512; i++)
	amp[i] = sqrtf(out[i].real * out[i].real + out[i].imag * out[i].imag) / 1024.0f;

fft_free(fft);
```
