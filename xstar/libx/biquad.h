#ifndef __XSTAR_LIBX_BIQUAD_H__
#define __XSTAR_LIBX_BIQUAD_H__

#ifdef __cplusplus
extern "C" {
#endif

enum biquad_filter_type_t {
	BIQUAD_FILTER_TYPE_LOWPASS,
	BIQUAD_FILTER_TYPE_HIGHPASS,
	BIQUAD_FILTER_TYPE_BANDPASS,
	BIQUAD_FILTER_TYPE_LOWSHELF,
	BIQUAD_FILTER_TYPE_HIGHSHELF,
	BIQUAD_FILTER_TYPE_PEAKING,
	BIQUAD_FILTER_TYPE_NOTCH,
	BIQUAD_FILTER_TYPE_ALLPASS,
};

struct biquad_filter_t {
	float b0, b1, b2;
	float a1, a2;
	float xn1, xn2;
	float yn1, yn2;
};

struct biquad_filter_t * biquad_alloc(enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain);
void biquad_free(struct biquad_filter_t * filter);
void biquad_init(struct biquad_filter_t * filter, enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain);
void biquad_clear(struct biquad_filter_t * filter);
void biquad_process(struct biquad_filter_t * filter, float * input, float * output, int len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_BIQUAD_H__ */
