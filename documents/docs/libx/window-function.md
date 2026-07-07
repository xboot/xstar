# 窗函数 (winfunc)

信号处理窗函数，用于 FFT 频谱分析时减少频谱泄漏。支持多种常用窗函数。

## API

```c
void winfunc_rectangular(float * taps, int n);
void winfunc_triangular(float * taps, int n);
void winfunc_flattop(float * taps, int n);
void winfunc_welch(float * taps, int n);
void winfunc_hanning(float * taps, int n);
void winfunc_hamming(float * taps, int n);
void winfunc_blackman(float * taps, int n);
void winfunc_kaiser(float * taps, int n, float beta);
```

- `winfunc_rectangular` — 矩形窗
- `winfunc_triangular` — 三角窗
- `winfunc_flattop` — 平顶窗
- `winfunc_welch` — Welch 窗
- `winfunc_hanning` — Hanning 窗
- `winfunc_hamming` — Hamming 窗
- `winfunc_blackman` — Blackman 窗
- `winfunc_kaiser` — Kaiser 窗，`beta` 控制旁瓣衰减

## 使用示例

```c
int n = 1024;
float window[n];
float signal[n];
float windowed[n];

winfunc_hanning(window, n);

for (int i = 0; i < n; i++)
	windowed[i] = signal[i] * window[i];

struct fft_t * fft = fft_new(10);
struct complex_t out[n];
fft_process_forward(fft, out, windowed);
fft_free(fft);
```
