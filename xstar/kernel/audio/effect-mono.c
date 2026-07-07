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
 * Mono - Mix all of channels to mono audio.
 *
 * Example:
 *   "mono": {
 *   }
 */

struct mono_pdata_t {
	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static void * mono_create(void)
{
	struct mono_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct mono_pdata_t));
	if(!pdat)
		return NULL;

	xos_memset(&pdat->output, 0, sizeof(struct audio_frame_t));
	pdat->samples = NULL;
	pdat->nsample = 0;

	return pdat;
}

static void mono_setup(void * ctx, struct json_value_t * v)
{
}

static void mono_prepare(void * ctx, struct audio_frame_t * input)
{
	struct mono_pdata_t * pdat = (struct mono_pdata_t *)ctx;
	int nsample = input->frames;

	if(nsample > pdat->nsample)
	{
		pdat->nsample = nsample;
		pdat->samples = xos_mem_realloc(pdat->samples, sizeof(float) * pdat->nsample);
	}
}

static struct audio_frame_t * mono_process(void * ctx, struct audio_frame_t * input)
{
	struct mono_pdata_t * pdat = (struct mono_pdata_t *)ctx;
	float * pi = input->samples;
	float * po = pdat->samples;

	for(int n = 0; n < input->frames; n++)
	{
		float tmp = 0;
		for(int c = 0; c < input->channel; c++)
			tmp += pi[c];
		po[0] = tmp / input->channel;
		pi += input->channel;
		po++;
	}
	pdat->output.rate = input->rate;
	pdat->output.channel = 1;
	pdat->output.frames = input->frames;
	pdat->output.samples = pdat->samples;

	return &pdat->output;
}

static void mono_destroy(void * ctx)
{
	struct mono_pdata_t * pdat = (struct mono_pdata_t *)ctx;

	if(pdat)
	{
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

static struct audio_effect_t mono = {
	.name		= "mono",
	.create		= mono_create,
	.setup		= mono_setup,
	.prepare	= mono_prepare,
	.process	= mono_process,
	.destroy	= mono_destroy,
};

static void effect_mono_init(void)
{
	register_audio_effect(&mono);
}

static void effect_mono_exit(void)
{
	unregister_audio_effect(&mono);
}

core_initcall(effect_mono_init);
core_exitcall(effect_mono_exit);
