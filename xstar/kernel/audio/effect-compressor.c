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
 * Compressor - Feed-forward dynamic range compressor with soft knee.
 *
 * Detects the peak across all channels per frame, computes the desired gain
 * reduction in the log domain using a configurable ratio and soft knee, then
 * smoothly tracks the envelope with separate attack and release time constants.
 * A single gain is shared across all channels to preserve the stereo image.
 *
 * Example:
 *   "compressor": {
 *       "threshold" : -20.0,
 *       "ratio"     : 4.0,
 *       "attack"    : 5.0,
 *       "release"   : 100.0,
 *       "knee"      : 6.0,
 *       "makeup"    : 0.0
 *   }
 *
 * threshold : Knee center in dBFS, range [-60.0, 0.0],   default -20.0 dBFS.
 * ratio     : Compression ratio N:1, range [1.0, 100.0], default 4.0.
 * attack    : Attack time in ms,     range [0.1, 1000.0],default 5.0 ms.
 * release   : Release time in ms,    range [1.0, 5000.0],default 100.0 ms.
 * knee      : Soft knee width in dB, range [0.0, 24.0],  default 6.0 dB.
 * makeup    : Make-up gain in dB,    range [-24.0, 24.0],default 0.0 dB.
 */

#define COMPRESSOR_FLOOR_DB			(-200.0f)

struct compressor_pdata_t {
	float threshold;
	float ratio;
	float attack;
	float release;
	float knee;
	float makeup;

	float env_db;
	float makeup_lin;
	float attack_coef;
	float release_coef;
	int __setup;
	int __rate;
};

static inline float compressor_lin_to_db(float x)
{
	if(x < 1.0e-10f)
		return COMPRESSOR_FLOOR_DB;
	return 20.0f * log10f(x);
}

static inline float compressor_db_to_lin(float db)
{
	return powf(10.0f, db / 20.0f);
}

static void * compressor_create(void)
{
	struct compressor_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct compressor_pdata_t));
	if(!pdat)
		return NULL;

	pdat->threshold = -20.0f;
	pdat->ratio = 4.0f;
	pdat->attack = 5.0f;
	pdat->release = 100.0f;
	pdat->knee = 6.0f;
	pdat->makeup = 0.0f;

	pdat->env_db = 0.0f;
	pdat->makeup_lin = 1.0f;
	pdat->attack_coef = 0.0f;
	pdat->release_coef = 0.0f;
	pdat->__setup = 1;
	pdat->__rate = 0;

	return pdat;
}

static void compressor_setup(void * ctx, struct json_value_t * v)
{
	struct compressor_pdata_t * pdat = (struct compressor_pdata_t *)ctx;

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
					pdat->__setup = 1;
				}
				break;

			case 0x104515c4: /* "ratio" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->ratio = XCLAMP((float)o->u.dbl, 1.0f, 100.0f);
					pdat->__setup = 1;
				}
				break;

			case 0xf297c9fd: /* "attack" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->attack = XCLAMP((float)o->u.dbl, 0.1f, 1000.0f);
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

			case 0x7c999d68: /* "knee" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->knee = XCLAMP((float)o->u.dbl, 0.0f, 24.0f);
					pdat->__setup = 1;
				}
				break;

			case 0x0d3903e8: /* "makeup" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->makeup = XCLAMP((float)o->u.dbl, -24.0f, 24.0f);
					pdat->makeup_lin = compressor_db_to_lin(pdat->makeup);
					pdat->__setup = 1;
				}
				break;

			default:
				break;
			}
		}
	}
}

static void compressor_prepare(void * ctx, struct audio_frame_t * input)
{
	struct compressor_pdata_t * pdat = (struct compressor_pdata_t *)ctx;

	if((input->rate != pdat->__rate) || pdat->__setup)
	{
		pdat->attack_coef = 1.0f - expf(-1000.0f / (pdat->attack * (float)input->rate));
		pdat->release_coef = 1.0f - expf(-1000.0f / (pdat->release * (float)input->rate));
		pdat->makeup_lin = compressor_db_to_lin(pdat->makeup);
		pdat->__setup = 0;
		pdat->__rate = input->rate;
	}
}

static struct audio_frame_t * compressor_process(void * ctx, struct audio_frame_t * input)
{
	struct compressor_pdata_t * pdat = (struct compressor_pdata_t *)ctx;
	float * pi = input->samples;
	float threshold = pdat->threshold;
	float knee = pdat->knee;
	float half_knee = knee * 0.5f;
	float slope = 1.0f - (1.0f / pdat->ratio);
	float attack_coef = pdat->attack_coef;
	float release_coef = pdat->release_coef;
	float makeup_lin = pdat->makeup_lin;
	float env_db = pdat->env_db;

	for(int n = 0; n < input->frames; n++)
	{
		float peak = 0.0f;
		for(int c = 0; c < input->channel; c++)
		{
			float a = fabsf(pi[c]);
			if(a > peak)
				peak = a;
		}
		float in_db = compressor_lin_to_db(peak);
		float over = in_db - threshold;
		float gr_db;
		if(knee > 0.0f && over > -half_knee && over < half_knee)
		{
			float t = over + half_knee;
			gr_db = slope * (t * t) / (2.0f * knee);
		}
		else if(over >= half_knee)
		{
			gr_db = slope * over;
		}
		else
		{
			gr_db = 0.0f;
		}
		float coef = (gr_db > env_db) ? attack_coef : release_coef;
		env_db += (gr_db - env_db) * coef;
		float gain = compressor_db_to_lin(-env_db) * makeup_lin;
		for(int c = 0; c < input->channel; c++)
			pi[c] = pi[c] * gain;
		pi += input->channel;
	}
	pdat->env_db = env_db;
	return input;
}

static void compressor_destroy(void * ctx)
{
	struct compressor_pdata_t * pdat = (struct compressor_pdata_t *)ctx;

	if(pdat)
		xos_mem_free(pdat);
}

static struct audio_effect_t compressor = {
	.name		= "compressor",
	.create		= compressor_create,
	.setup		= compressor_setup,
	.prepare	= compressor_prepare,
	.process	= compressor_process,
	.destroy	= compressor_destroy,
};

static void effect_compressor_init(void)
{
	register_audio_effect(&compressor);
}

static void effect_compressor_exit(void)
{
	unregister_audio_effect(&compressor);
}

core_initcall(effect_compressor_init);
core_exitcall(effect_compressor_exit);
