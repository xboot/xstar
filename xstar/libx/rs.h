#ifndef __XSTAR_LIBX_RS_H__
#define __XSTAR_LIBX_RS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

/*
 * Reed-Solomon Forward Error Correction (FEC)
 */
struct rsctx_t {
	int mm;						/* Bits per symbol */
	int nn;						/* Symbols per block = (1 << mm) - 1 */
	unsigned char * alpha_to;	/* log lookup table */
	unsigned char * index_of;	/* Antilog lookup table */
	unsigned char * genpoly;	/* Generator polynomial */
	int nroots;					/* Number of generator roots = number of parity symbols */
	int fcr;					/* First consecutive root, index form */
	int prim;					/* Primitive element, index form */
	int iprim;					/* prim-th root of 1, index form */
};

struct rsctx_t * rsctx_alloc(int nroots);
void rsctx_free(struct rsctx_t * ctx);

int rsctx_encode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity);
int rsctx_decode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_RS_H__ */
