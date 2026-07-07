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

#include <xos/xos.h>
#include <libx/rs.h>

static inline int modnn(struct rsctx_t * ctx, int x)
{
	while(x >= ctx->nn)
	{
		x -= ctx->nn;
		x = (x >> ctx->mm) + (x & ctx->nn);
	}
	return x;
}

static struct rsctx_t * rsctx_alloc_ex(int symsize, int gfpoly, int fcr, int prim, int nroots)
{
	struct rsctx_t * ctx;

	if(symsize < 0 || symsize > 8 * (int)sizeof(unsigned char))
		return NULL;
	if(fcr < 0 || fcr >= (1 << symsize))
		return NULL;
	if(prim <= 0 || prim >= (1 << symsize))
		return NULL;
	if(nroots < 0 || nroots >= (1 << symsize))
		return NULL;

	ctx = xos_mem_malloc(sizeof(struct rsctx_t));
	if(ctx == NULL)
		return NULL;

	ctx->mm = symsize;
	ctx->nn = (1 << symsize) - 1;

	ctx->alpha_to = xos_mem_malloc(sizeof(unsigned char) * (ctx->nn + 1));
	if(!ctx->alpha_to)
	{
		xos_mem_free(ctx);
		return NULL;
	}
	ctx->index_of = xos_mem_malloc(sizeof(unsigned char) * (ctx->nn + 1));
	if(!ctx->index_of)
	{
		xos_mem_free(ctx->alpha_to);
		xos_mem_free(ctx);
		return NULL;
	}
	xos_memset(ctx->index_of, 0, sizeof(unsigned char) * (ctx->nn + 1));

	ctx->index_of[0] = ctx->nn;
	ctx->alpha_to[ctx->nn] = 0;
	int sr = 1;
	for(int i = 0; i < ctx->nn; i++)
	{
		ctx->index_of[sr] = i;
		ctx->alpha_to[i] = sr;
		sr <<= 1;
		if(sr & (1 << symsize))
			sr ^= gfpoly;
		sr &= ctx->nn;
	}
	if(sr != 1)
	{
		xos_mem_free(ctx->alpha_to);
		xos_mem_free(ctx->index_of);
		xos_mem_free(ctx);
		return NULL;
	}

	ctx->genpoly = xos_mem_malloc(sizeof(unsigned char) * (nroots + 1));
	if(!ctx->genpoly)
	{
		xos_mem_free(ctx->alpha_to);
		xos_mem_free(ctx->index_of);
		xos_mem_free(ctx);
		return NULL;
	}

	ctx->nroots = nroots;
	ctx->fcr = fcr;
	ctx->prim = prim;

	int iprim = 1;
	for(; (iprim % prim) != 0; iprim += ctx->nn);
	ctx->iprim = iprim / prim;

	ctx->genpoly[0] = 1;
	for(int i = 0, root = fcr * prim; i < nroots; i++, root += prim)
	{
		ctx->genpoly[i + 1] = 1;
		for(int j = i; j > 0; j--)
		{
			if(ctx->genpoly[j] != 0)
				ctx->genpoly[j] = ctx->genpoly[j - 1] ^ ctx->alpha_to[modnn(ctx, ctx->index_of[ctx->genpoly[j]] + root)];
			else
				ctx->genpoly[j] = ctx->genpoly[j - 1];
		}
		ctx->genpoly[0] = ctx->alpha_to[modnn(ctx, ctx->index_of[ctx->genpoly[0]] + root)];
	}
	for(int i = 0; i <= nroots; i++)
		ctx->genpoly[i] = ctx->index_of[ctx->genpoly[i]];
	return ctx;
}

struct rsctx_t * rsctx_alloc(int nroots)
{
    return rsctx_alloc_ex(8, 0x11d, 0, 1, nroots);
}

void rsctx_free(struct rsctx_t * ctx)
{
	if(ctx)
	{
		if(ctx->alpha_to)
			xos_mem_free(ctx->alpha_to);
		if(ctx->index_of)
			xos_mem_free(ctx->index_of);
		if(ctx->genpoly)
			xos_mem_free(ctx->genpoly);
		xos_mem_free(ctx);
	}
}

int rsctx_encode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity)
{
	int pad = ctx->nn - ctx->nroots - len;
	if(pad < 0 || pad >= ctx->nn)
		return 0;
	xos_memset(parity, 0, ctx->nroots * sizeof(unsigned char));
	for(int i = 0; i < len; i++)
	{
		unsigned char feedback = ctx->index_of[data[i] ^ parity[0]];
		if(feedback != ctx->nn)
		{
			for(int j = 1; j < ctx->nroots; j++)
				parity[j] ^= ctx->alpha_to[modnn(ctx, feedback + ctx->genpoly[ctx->nroots - j])];
		}
		xos_memmove(&parity[0], &parity[1], sizeof(unsigned char) * (ctx->nroots - 1));
		if(feedback != ctx->nn)
			parity[ctx->nroots - 1] = ctx->alpha_to[modnn(ctx, feedback + ctx->genpoly[0])];
		else
			parity[ctx->nroots - 1] = 0;
	}
	return 1;
}

int rsctx_decode(struct rsctx_t * ctx, unsigned char * data, unsigned int len, unsigned char * parity)
{
	unsigned char lambda[ctx->nroots + 1];
	unsigned char s[ctx->nroots + 1];
	unsigned char b[ctx->nroots + 1];
	unsigned char t[ctx->nroots + 1];
	unsigned char omega[ctx->nroots + 1];
	unsigned char root[ctx->nroots + 1];
	unsigned char reg[ctx->nroots + 1];
	unsigned char loc[ctx->nroots + 1];

	int pad = ctx->nn - ctx->nroots - len;
	if(pad < 0 || pad >= ctx->nn)
		return 0;

	for(int i = 0; i < ctx->nroots; i++)
		s[i] = data[0];
	for(int j = 1; j < len; j++)
	{
		for(int i = 0; i < ctx->nroots; i++)
		{
			if(s[i] == 0)
				s[i] = data[j];
			else
				s[i] = data[j] ^ ctx->alpha_to[modnn(ctx, ctx->index_of[s[i]] + (ctx->fcr + i) * ctx->prim)];
		}
	}
	for(int j = 0; j < ctx->nroots; j++)
	{
		for(int i = 0; i < ctx->nroots; i++)
		{
			if(s[i] == 0)
				s[i] = parity[j];
			else
				s[i] = parity[j] ^ ctx->alpha_to[modnn(ctx, ctx->index_of[s[i]] + (ctx->fcr + i) * ctx->prim)];
		}
	}

	int syn_error = 0;
	for(int i = 0; i < ctx->nroots; i++)
	{
		syn_error |= s[i];
		s[i] = ctx->index_of[s[i]];
	}
	if(!syn_error)
		return 1;

	xos_memset(&lambda[1], 0, ctx->nroots * sizeof(lambda[0]));
	lambda[0] = 1;
	for(int i = 0; i < ctx->nroots + 1; i++)
		b[i] = ctx->index_of[lambda[i]];

	int r = 0;
	int el = 0;
	while(++r <= ctx->nroots)
	{
		unsigned char discr_r = 0;
		for(int i = 0; i < r; i++)
		{
			if((lambda[i] != 0) && (s[r - i - 1] != ctx->nn))
				discr_r ^= ctx->alpha_to[modnn(ctx, ctx->index_of[lambda[i]] + s[r - i - 1])];
		}
		discr_r = ctx->index_of[discr_r];
		if(discr_r == ctx->nn)
		{
			xos_memmove(&b[1], b, ctx->nroots * sizeof(b[0]));
			b[0] = ctx->nn;
		}
		else
		{
			t[0] = lambda[0];
			for(int i = 0; i < ctx->nroots; i++)
			{
				if(b[i] != ctx->nn)
					t[i + 1] = lambda[i + 1] ^ ctx->alpha_to[modnn(ctx, discr_r + b[i])];
				else
					t[i + 1] = lambda[i + 1];
			}
			if(2 * el <= r - 1)
			{
				el = r - el;
				for(int i = 0; i <= ctx->nroots; i++)
					b[i] = (lambda[i] == 0) ? ctx->nn : modnn(ctx, ctx->index_of[lambda[i]] - discr_r + ctx->nn);
			}
			else
			{
				xos_memmove(&b[1], b, ctx->nroots * sizeof(b[0]));
				b[0] = ctx->nn;
			}
			xos_memcpy(lambda, t, (ctx->nroots + 1) * sizeof(t[0]));
		}
	}

	int deg_lambda = 0;
	for(int i = 0; i < ctx->nroots + 1; i++)
	{
		lambda[i] = ctx->index_of[lambda[i]];
		if(lambda[i] != ctx->nn)
			deg_lambda = i;
	}
	if(deg_lambda == 0)
		return 0;

	xos_memcpy(&reg[1], &lambda[1], ctx->nroots * sizeof(reg[0]));
	int count = 0;
	for(int i = 1, k = ctx->iprim - 1; i <= ctx->nn; i++, k = modnn(ctx, k + ctx->iprim))
	{
		unsigned char q = 1;
		for(int j = deg_lambda; j > 0; j--)
		{
			if(reg[j] != ctx->nn)
			{
				reg[j] = modnn(ctx, reg[j] + j);
				q ^= ctx->alpha_to[reg[j]];
			}
		}
		if(q != 0)
			continue;
		if(k < pad)
			return 0;
		root[count] = i;
		loc[count] = k;
		if(++count == deg_lambda)
			break;
	}
	if(deg_lambda != count)
		return 0;

	int deg_omega = deg_lambda - 1;
	for(int i = 0; i <= deg_omega; i++)
	{
		unsigned char tmp = 0;
		for(int j = i; j >= 0; j--)
		{
			if((s[i - j] != ctx->nn) && (lambda[j] != ctx->nn))
				tmp ^= ctx->alpha_to[modnn(ctx, s[i - j] + lambda[j])];
		}
		omega[i] = ctx->index_of[tmp];
	}
	for(int j = count - 1; j >= 0; j--)
	{
		unsigned char num1 = 0;
		for(int i = deg_omega; i >= 0; i--)
		{
			if(omega[i] != ctx->nn)
				num1 ^= ctx->alpha_to[modnn(ctx, omega[i] + i * root[j])];
		}
		if(num1 == 0)
		{
			b[j] = 0;
			continue;
		}
		unsigned char num2 = ctx->alpha_to[modnn(ctx, root[j] * (ctx->fcr - 1) + ctx->nn)];
		unsigned char den = 0;
		for(int i = XMIN(deg_lambda, ctx->nroots - 1) & ~1; i >= 0; i -= 2)
		{
			if(lambda[i + 1] != ctx->nn)
				den ^= ctx->alpha_to[modnn(ctx, lambda[i + 1] + i * root[j])];
		}
		b[j] = ctx->alpha_to[modnn(ctx, ctx->index_of[num1] + ctx->index_of[num2] + ctx->nn - ctx->index_of[den])];
	}
	for(int i = 0; i < ctx->nroots; i++)
	{
		unsigned char tmp = 0;
		for(int j = 0; j < count; j++)
		{
			if(b[j] == 0)
				continue;
			int k = (ctx->fcr + i) * ctx->prim * (ctx->nn - loc[j] - 1);
			tmp ^= ctx->alpha_to[modnn(ctx, ctx->index_of[b[j]] + k)];
		}
		if(tmp != ctx->alpha_to[s[i]])
			return 0;
	}
	for(int i = 0; i < count; i++)
	{
		if(loc[i] < (ctx->nn - ctx->nroots))
			data[loc[i] - pad] ^= b[i];
		else
			parity[loc[i] - pad - len] ^= b[i];
	}
	return 1;
}
