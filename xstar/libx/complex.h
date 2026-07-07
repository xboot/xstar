#ifndef __XSTAR_LIBX_COMPLEX_H__
#define __XSTAR_LIBX_COMPLEX_H__

#ifdef __cplusplus
extern "C" {
#endif

struct complex_t {
	float real;
	float imag;
};

void complex_add(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_sub(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_mul(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_div(struct complex_t * z, struct complex_t * a, struct complex_t * b);
void complex_neg(struct complex_t * z, struct complex_t * a);
void complex_conj(struct complex_t * z, struct complex_t * a);
void complex_expj(struct complex_t * z, float phi);
void complex_proj(struct complex_t * z, struct complex_t * a);
void complex_normalize(struct complex_t * z, struct complex_t * a);
float complex_abs(struct complex_t * z);
float complex_arg(struct complex_t * z);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_COMPLEX_H__ */
