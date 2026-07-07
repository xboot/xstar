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
 * Duplicate - Just duplicate input audio frame and buffered.
 *
 * Example:
 *   "duplicate": {
 *   }
 */

struct duplicate_pdata_t {
	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static void * duplicate_create(void)
{
	struct duplicate_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct duplicate_pdata_t));
	if(!pdat)
		return NULL;

	xos_memset(&pdat->output, 0, sizeof(struct audio_frame_t));
	pdat->samples = NULL;
	pdat->nsample = 0;

	return pdat;
}

static void duplicate_setup(void * ctx, struct json_value_t * v)
{
}

static void duplicate_prepare(void * ctx, struct audio_frame_t * input)
{
	struct duplicate_pdata_t * pdat = (struct duplicate_pdata_t *)ctx;
	int nsample = input->channel * input->frames;

	if(nsample > pdat->nsample)
	{
		pdat->nsample = nsample;
		pdat->samples = xos_mem_realloc(pdat->samples, sizeof(float) * pdat->nsample);
	}
}

static struct audio_frame_t * duplicate_process(void * ctx, struct audio_frame_t * input)
{
	struct duplicate_pdata_t * pdat = (struct duplicate_pdata_t *)ctx;

	xos_memcpy(pdat->samples, input->samples, sizeof(float) * input->channel * input->frames);
	pdat->output.rate = input->rate;
	pdat->output.channel = input->channel;
	pdat->output.frames = input->frames;
	pdat->output.samples = pdat->samples;

	return &pdat->output;
}

static void duplicate_destroy(void * ctx)
{
	struct duplicate_pdata_t * pdat = (struct duplicate_pdata_t *)ctx;

	if(pdat)
	{
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

static struct audio_effect_t duplicate = {
	.name		= "duplicate",
	.create		= duplicate_create,
	.setup		= duplicate_setup,
	.prepare	= duplicate_prepare,
	.process	= duplicate_process,
	.destroy	= duplicate_destroy,
};

static void effect_duplicate_init(void)
{
	register_audio_effect(&duplicate);
}

static void effect_duplicate_exit(void)
{
	unregister_audio_effect(&duplicate);
}

core_initcall(effect_duplicate_init);
core_exitcall(effect_duplicate_exit);
