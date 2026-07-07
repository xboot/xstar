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
 * Limiter - Peak limiter to prevent samples from exceeding a threshold.
 *
 * Reacts instantly when the input peak exceeds the threshold (zero attack)
 * and recovers smoothly back to unity gain over the configured release time.
 * All channels share a single gain so stereo imaging is preserved.
 *
 * Example:
 *   "limiter": {
 *       "threshold" : -1.0,
 *       "release"   : 50.0
 *   }
 *
 * threshold : Ceiling in dBFS (dB relative to Full Scale, i.e. ±1.0 sample range),
 *             range [-60.0, 0.0], default -1.0 dBFS.
 * release   : Release time in milliseconds, range [1.0, 5000.0], default 50 ms.
 */

struct limiter_pdata_t {
	float threshold;
	float release;

	float ceiling;
	float gain;
	float coef;
	int __setup;
	int __rate;
};

static void * limiter_create(void)
{
	struct limiter_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct limiter_pdata_t));
	if(!pdat)
		return NULL;

	pdat->threshold = -1.0f;
	pdat->release = 50.0f;

	pdat->ceiling = powf(10.0f, pdat->threshold / 20.0f);
	pdat->gain = 1.0f;
	pdat->coef = 0.0f;
	pdat->__setup = 1;
	pdat->__rate = 0;

	return pdat;
}

static void limiter_setup(void * ctx, struct json_value_t * v)
{
	struct limiter_pdata_t * pdat = (struct limiter_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0xf0a23fd2: /* "threshold" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->threshold = XCLAMP((float)o->u.dbl, -60.0f, 0.0f);
					pdat->ceiling = powf(10.0f, pdat->threshold / 20.0f);
					pdat->__setup = 1;
				}
				break;

			case 0x3ea31766: /* "release" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->release = XCLAMP((float)o->u.dbl, 1.0f, 5000.0f);
					pdat->__setup = 1;
				}
				break;

			default:
				break;
			}
		}
	}
}

static void limiter_prepare(void * ctx, struct audio_frame_t * input)
{
	struct limiter_pdata_t * pdat = (struct limiter_pdata_t *)ctx;

	if((input->rate != pdat->__rate) || pdat->__setup)
	{
		pdat->coef = 1.0f - expf(-1000.0f / (pdat->release * (float)input->rate));
		pdat->__setup = 0;
		pdat->__rate = input->rate;
	}
}

static struct audio_frame_t * limiter_process(void * ctx, struct audio_frame_t * input)
{
	struct limiter_pdata_t * pdat = (struct limiter_pdata_t *)ctx;
	float * pi = input->samples;
	float ceiling = pdat->ceiling;
	float coef = pdat->coef;
	float gain = pdat->gain;

	for(int n = 0; n < input->frames; n++)
	{
		float peak = 0.0f;
		for(int c = 0; c < input->channel; c++)
		{
			float a = fabsf(pi[c]);
			if(a > peak)
				peak = a;
		}
		if(peak * gain > ceiling)
			gain = ceiling / peak;
		else
			gain += (1.0f - gain) * coef;
		for(int c = 0; c < input->channel; c++)
			pi[c] = pi[c] * gain;
		pi += input->channel;
	}
	pdat->gain = gain;
	return input;
}

static void limiter_destroy(void * ctx)
{
	struct limiter_pdata_t * pdat = (struct limiter_pdata_t *)ctx;

	if(pdat)
		xos_mem_free(pdat);
}

static struct audio_effect_t limiter = {
	.name		= "limiter",
	.create		= limiter_create,
	.setup		= limiter_setup,
	.prepare	= limiter_prepare,
	.process	= limiter_process,
	.destroy	= limiter_destroy,
};

static void effect_limiter_init(void)
{
	register_audio_effect(&limiter);
}

static void effect_limiter_exit(void)
{
	unregister_audio_effect(&limiter);
}

core_initcall(effect_limiter_init);
core_exitcall(effect_limiter_exit);
