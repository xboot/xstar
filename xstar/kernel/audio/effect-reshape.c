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
 * Reshape - Change the number of audio channels.
 *
 * Example:
 *   "reshape": {
 *       "channel" : 1
 *   }
 */

struct reshape_pdata_t {
	struct audio_frame_t output;
	int channel;
	float * samples;
	int nsample;
};

static void * reshape_create(void)
{
	struct reshape_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct reshape_pdata_t));
	if(!pdat)
		return NULL;

	xos_memset(&pdat->output, 0, sizeof(struct audio_frame_t));
	pdat->channel = 1;
	pdat->samples = NULL;
	pdat->nsample = 0;

	return pdat;
}

static void reshape_setup(void * ctx, struct json_value_t * v)
{
	struct reshape_pdata_t * pdat = (struct reshape_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0xc237195e: /* "channel" */
				if(o && (o->type == JSON_INTEGER))
					pdat->channel = XMAX((int)o->u.integer, 1);
				break;

			default:
				break;
			}
		}
	}
}

static void reshape_prepare(void * ctx, struct audio_frame_t * input)
{
	struct reshape_pdata_t * pdat = (struct reshape_pdata_t *)ctx;
	int nsample = pdat->channel * input->frames;

	if(nsample > pdat->nsample)
	{
		pdat->nsample = nsample;
		pdat->samples = xos_mem_realloc(pdat->samples, sizeof(float) * pdat->nsample);
	}
}

static struct audio_frame_t * reshape_process(void * ctx, struct audio_frame_t * input)
{
	struct reshape_pdata_t * pdat = (struct reshape_pdata_t *)ctx;
	float * pi = input->samples;
	float * po = pdat->samples;

	for(int n = 0; n < input->frames; n++)
	{
		float tmp = 0;
		int ch = 0;
		for(; ch < XMIN(input->channel, pdat->channel); ch++)
			tmp = po[ch] = pi[ch];
		for(int c = ch; c < pdat->channel; c++)
			po[c] = tmp;
		pi += input->channel;
		po += pdat->channel;
	}
	pdat->output.rate = input->rate;
	pdat->output.channel = pdat->channel;
	pdat->output.frames = input->frames;
	pdat->output.samples = pdat->samples;

	return &pdat->output;
}

static void reshape_destroy(void * ctx)
{
	struct reshape_pdata_t * pdat = (struct reshape_pdata_t *)ctx;

	if(pdat)
	{
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

static struct audio_effect_t reshape = {
	.name		= "reshape",
	.create		= reshape_create,
	.setup		= reshape_setup,
	.prepare	= reshape_prepare,
	.process	= reshape_process,
	.destroy	= reshape_destroy,
};

static void effect_reshape_init(void)
{
	register_audio_effect(&reshape);
}

static void effect_reshape_exit(void)
{
	unregister_audio_effect(&reshape);
}

core_initcall(effect_reshape_init);
core_exitcall(effect_reshape_exit);
