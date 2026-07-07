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

#include <kernel/audio/effect.h>

/*
 * Crystalizer - Simple audio noise sharpening filter.
 *
 * Example:
 *   "crystalizer": {
 *       "intensity" : 2.0,
 *       "clip" : true
 *   }
 */

struct crystalizer_pdata_t {
	float intensity;
	int clip;
	float * buf;
	int nbuf;
};

static void * crystalizer_create(void)
{
	struct crystalizer_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct crystalizer_pdata_t));
	if(!pdat)
		return NULL;

	pdat->intensity = 2.0f;
	pdat->clip = 1;
	pdat->buf = NULL;
	pdat->nbuf = 0;

	return pdat;
}

static void crystalizer_setup(void * ctx, struct json_value_t * v)
{
	struct crystalizer_pdata_t * pdat = (struct crystalizer_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0x127450cc: /* "intensity" */
				if(o && (o->type == JSON_DOUBLE))
					pdat->intensity = XCLAMP((float)o->u.dbl, -10.0f, 10.0f);
				break;

			case 0x7c95326d: /* "clip" */
				if(o && (o->type == JSON_BOOLEAN))
					pdat->clip = o->u.boolean ? 1 : 0;
				break;

			default:
				break;
			}
		}
	}
}

static void crystalizer_prepare(void * ctx, struct audio_frame_t * input)
{
	struct crystalizer_pdata_t * pdat = (struct crystalizer_pdata_t *)ctx;

	if(input->channel > pdat->nbuf)
	{
		pdat->nbuf = input->channel;
		pdat->buf = xos_mem_realloc(pdat->buf, sizeof(float) * pdat->nbuf);
		xos_memset(pdat->buf, 0, sizeof(float) * pdat->nbuf);
	}
}

static struct audio_frame_t * crystalizer_process(void * ctx, struct audio_frame_t * input)
{
	struct crystalizer_pdata_t * pdat = (struct crystalizer_pdata_t *)ctx;
	float * pi = input->samples;

	if(pdat->intensity >= 0.0f)
	{
		for(int n = 0; n < input->frames; n++)
		{
			for(int c = 0; c < input->channel; c++)
			{
				float v = pi[c];
				pi[c] = v + (v - pdat->buf[c]) * pdat->intensity;
				if(pdat->clip)
					pi[c] = XCLAMP(pi[c], -1.0f, 1.0f);
				pdat->buf[c] = v;
			}
			pi += input->channel;
		}
	}
	else
	{
		for(int n = 0; n < input->frames; n++)
		{
			for(int c = 0; c < input->channel; c++)
			{
				float v = pi[c];
				pi[c] = (v - pdat->buf[c] * pdat->intensity) / (1.0f - pdat->intensity);
				if(pdat->clip)
					pi[c] = XCLAMP(pi[c], -1.0f, 1.0f);
				pdat->buf[c] = pi[c];
			}
			pi += input->channel;
		}
	}
	return input;
}

static void crystalizer_destroy(void * ctx)
{
	struct crystalizer_pdata_t * pdat = (struct crystalizer_pdata_t *)ctx;

	if(pdat)
	{
		if(pdat->buf)
			xos_mem_free(pdat->buf);
		xos_mem_free(pdat);
	}
}

static struct audio_effect_t crystalizer = {
	.name		= "crystalizer",
	.create		= crystalizer_create,
	.setup		= crystalizer_setup,
	.prepare	= crystalizer_prepare,
	.process	= crystalizer_process,
	.destroy	= crystalizer_destroy,
};

static void effect_crystalizer_init(void)
{
	register_audio_effect(&crystalizer);
}

static void effect_crystalizer_exit(void)
{
	unregister_audio_effect(&crystalizer);
}

core_initcall(effect_crystalizer_init);
core_exitcall(effect_crystalizer_exit);
