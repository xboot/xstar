/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __XAF_H__
#define __XAF_H__

#include <xstar.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cinepak_cv_t {
	uint32_t rgba[4];
};

struct cinepak_ctx_t {
	struct cinepak_cv_t v1[256];
	struct cinepak_cv_t v4[256];
	uint16_t width, height;
	uint32_t * frame;
	uint32_t fsize;
};

struct xaf_t {
	uint16_t width;
	uint16_t height;
	uint32_t nframes;
	uint8_t fps;
	uint32_t findex;
	uint8_t * data;
	uint32_t size;
	uint32_t pos;
	uint32_t * frame;
	uint32_t fsize;
	struct cinepak_ctx_t cinepak;
};

static inline uint16_t xaf_get_width(struct xaf_t * ctx)
{
	return ctx ? ctx->width : 0;
}

static inline uint16_t xaf_get_height(struct xaf_t * ctx)
{
	return ctx ? ctx->height : 0;
}

static inline uint8_t xaf_get_fps(struct xaf_t * ctx)
{
	return ctx ? ctx->fps : 0;
}

static inline uint32_t xaf_get_nframes(struct xaf_t * ctx)
{
	return ctx ? ctx->nframes : 0;
}

static inline uint32_t xaf_get_findex(struct xaf_t * ctx)
{
	return ctx ? ctx->findex : 0;
}

struct xaf_t * xaf_alloc(uint8_t * data, uint32_t size);
void xaf_free(struct xaf_t * ctx);
void xaf_reset(struct xaf_t * ctx);
uint32_t * xaf_next(struct xaf_t * ctx);

#ifdef __cplusplus
}
#endif

#endif /* __XAF_H__ */
