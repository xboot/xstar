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
 * Volume - Adjust audio volume filter.
 *
 * Example of factor:
 *   "volume": {
 *       "factor" : 0.5
 *   }
 *
 * Example of decibel:
 *   "volume": {
 *       "decibel" : -6.0
 *   }
 */

struct volume_pdata_t {
	float factor;
};

static void * volume_create(void)
{
	struct volume_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct volume_pdata_t));
	if(!pdat)
		return NULL;

	pdat->factor = 1.0;
	return pdat;
}

static void volume_setup(void * ctx, struct json_value_t * v)
{
	struct volume_pdata_t * pdat = (struct volume_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0xfce0c384: /* "factor" */
				if(o && (o->type == JSON_DOUBLE))
					pdat->factor = XMAX((float)o->u.dbl, 0.0f);
				break;

			case 0x08531c4d: /* "decibel" */
				if(o && (o->type == JSON_DOUBLE))
					pdat->factor = powf(10.0f, (float)o->u.dbl / 20.0f);
				break;

			default:
				break;
			}
		}
	}
}

static void volume_prepare(void * ctx, struct audio_frame_t * input)
{
}

static struct audio_frame_t * volume_process(void * ctx, struct audio_frame_t * input)
{
	struct volume_pdata_t * pdat = (struct volume_pdata_t *)ctx;
	float * pi = input->samples;

	for(int n = 0; n < input->frames; n++)
	{
		for(int c = 0; c < input->channel; c++)
			pi[c] = pi[c] * pdat->factor;
		pi += input->channel;
	}
	return input;
}

static void volume_destroy(void * ctx)
{
	struct volume_pdata_t * pdat = (struct volume_pdata_t *)ctx;

	if(pdat)
		xos_mem_free(pdat);
}

static struct audio_effect_t volume = {
	.name		= "volume",
	.create		= volume_create,
	.setup		= volume_setup,
	.prepare	= volume_prepare,
	.process	= volume_process,
	.destroy	= volume_destroy,
};

static void effect_volume_init(void)
{
	register_audio_effect(&volume);
}

static void effect_volume_exit(void)
{
	unregister_audio_effect(&volume);
}

core_initcall(effect_volume_init);
core_exitcall(effect_volume_exit);
