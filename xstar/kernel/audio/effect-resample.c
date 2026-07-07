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
 * The above copyright duplicatece and this permission notice shall be included in all
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
 * Resample - Change audio sample rate.
 *
 * Example:
 *   "resample": {
 *       "rate" : 48000
 *   }
 */

struct resample_pdata_t {
	struct audio_frame_t output;
	int rate;
	float * samples;
	int nsample;
	float fraction;
};

static void * resample_create(void)
{
	struct resample_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct resample_pdata_t));
	if(!pdat)
		return NULL;

	xos_memset(&pdat->output, 0, sizeof(struct audio_frame_t));
	pdat->rate = 48000;
	pdat->samples = NULL;
	pdat->nsample = 0;
	pdat->fraction = 0;

	return pdat;
}

static void resample_setup(void * ctx, struct json_value_t * v)
{
	struct resample_pdata_t * pdat = (struct resample_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0x7c9d3eb1: /* "rate" */
				if(o && (o->type == JSON_INTEGER))
					pdat->rate = XMAX((int)o->u.integer, 1000);
				break;

			default:
				break;
			}
		}
	}
}

static void resample_prepare(void * ctx, struct audio_frame_t * input)
{
	struct resample_pdata_t * pdat = (struct resample_pdata_t *)ctx;
	int nsample = (int)rint(input->frames * (float)pdat->rate / (float)input->rate + 0.5f) * input->channel;

	if(nsample > pdat->nsample)
	{
		pdat->nsample = nsample;
		pdat->samples = xos_mem_realloc(pdat->samples, sizeof(float) * pdat->nsample);
	}
}

static struct audio_frame_t * resample_process(void * ctx, struct audio_frame_t * input)
{
	struct resample_pdata_t * pdat = (struct resample_pdata_t *)ctx;
	float step = (float)input->rate / (float)pdat->rate;
	float * pi = input->samples;
	float * po = pdat->samples;
	int frames = 0;

	for(int n = 0; n < input->frames; n++)
	{
		while(pdat->fraction > 1.0f)
		{
			for(int c = 0; c < input->channel; c++)
				po[c] = pi[c];
			pdat->fraction -= step;
			po += input->channel;
			frames++;
		}
		pdat->fraction++;
		pi += input->channel;
	}
	pdat->output.rate = pdat->rate;
	pdat->output.channel = input->channel;
	pdat->output.frames = frames;
	pdat->output.samples = pdat->samples;

	return &pdat->output;
}

static void resample_destroy(void * ctx)
{
	struct resample_pdata_t * pdat = (struct resample_pdata_t *)ctx;

	if(pdat)
	{
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

static struct audio_effect_t resample = {
	.name		= "resample",
	.create		= resample_create,
	.setup		= resample_setup,
	.prepare	= resample_prepare,
	.process	= resample_process,
	.destroy	= resample_destroy,
};

static void effect_resample_init(void)
{
	register_audio_effect(&resample);
}

static void effect_resample_exit(void)
{
	unregister_audio_effect(&resample);
}

core_initcall(effect_resample_init);
core_exitcall(effect_resample_exit);
