#ifndef __XSTAR_LIBX_INTERLEAVER_H__
#define __XSTAR_LIBX_INTERLEAVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

struct interleaver_t {
	int size;
	int * pattern;
	int * ipattern;
};

struct interleaver_t * interleaver_alloc(int size, int seed);
void interleaver_free(struct interleaver_t * ctx);

void interleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output);
void deinterleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_INTERLEAVER_H__ */
