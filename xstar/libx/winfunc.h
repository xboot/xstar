#ifndef __XSTAR_LIBX_WINFUNC_H__
#define __XSTAR_LIBX_WINFUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

void winfunc_rectangular(float * taps, int n);
void winfunc_triangular(float * taps, int n);
void winfunc_flattop(float * taps, int n);
void winfunc_welch(float * taps, int n);
void winfunc_hanning(float * taps, int n);
void winfunc_hamming(float * taps, int n);
void winfunc_blackman(float * taps, int n);
void winfunc_kaiser(float * taps, int n, float beta);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_WINFUNC_H__ */
