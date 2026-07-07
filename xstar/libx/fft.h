#ifndef __XSTAR_LIBX_FFT_H__
#define __XSTAR_LIBX_FFT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/complex.h>

struct fft_t {
	struct complex_t * interleave;
	struct complex_t * lut;
	unsigned int * bitinverse;
	unsigned int size;
};

struct fft_t * fft_new(int size_log2);
void fft_free(struct fft_t * fft);
void fft_process_forward_complex(struct fft_t * fft, struct complex_t * out, struct complex_t * in);
void fft_process_forward(struct fft_t * fft, struct complex_t * out, float * in);
void fft_process_inverse(struct fft_t * fft, float * out, struct complex_t * in);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_FFT_H__ */
