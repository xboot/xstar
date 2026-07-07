# Window Function (winfunc)

Signal processing window functions for reducing spectral leakage in FFT analysis.

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

- `winfunc_rectangular` — Rectangular (uniform) window, no attenuation
- `winfunc_triangular` — Triangular (Bartlett) window
- `winfunc_flattop` — Flat-top window, optimized for amplitude accuracy
- `winfunc_welch` — Welch (parabolic) window
- `winfunc_hanning` — Hanning (raised cosine) window
- `winfunc_hamming` — Hamming window, modified Hanning with non-zero endpoints
- `winfunc_blackman` — Blackman window, higher side-lobe attenuation than Hanning/Hamming
- `winfunc_kaiser` — Kaiser window with configurable `beta` parameter controlling the trade-off between main-lobe width and side-lobe attenuation

All functions fill `taps` array with `n` window coefficients. Apply window by multiplying sample data element-wise with the taps array before FFT.

## Example

```c
int n = 1024;
float samples[n];
float window[n];

winfunc_hanning(window, n);

for(int i = 0; i < n; i++)
    samples[i] *= window[i];
```
