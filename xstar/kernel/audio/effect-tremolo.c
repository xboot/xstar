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
 * Tremolo - Tremolo effect.
 *
 * Example:
 *   "tremolo": {
 *       "frequency" : 5.0,
 *       "depth" : 0.5
 *   }
 */

struct tremolo_pdata_t {
	float frequency;
	float depth;
	float * table;
	int ntable;
	int index;
	int __setup;
	int __rate;
};

static void * tremolo_create(void)
{
	struct tremolo_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct tremolo_pdata_t));
	if(!pdat)
		return NULL;

	pdat->frequency = 5.0f;
	pdat->depth = 0.5;
	pdat->table = NULL;
	pdat->ntable = 0;
	pdat->index = 0;
	pdat->__setup = 0;
	pdat->__rate = 0;

	return pdat;
}

static void tremolo_setup(void * ctx, struct json_value_t * v)
{
	struct tremolo_pdata_t * pdat = (struct tremolo_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0xffd1bad7: /* "frequency" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->frequency = XCLAMP((float)o->u.dbl, 0.1f, 20000.0f);
					pdat->__setup = 1;
				}
				break;

			case 0x0f49e09a: /* "depth" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->depth = XCLAMP((float)o->u.dbl, 0.0f, 1.0f);
					pdat->__setup = 1;
				}
				break;

			default:
				break;
			}
		}
	}
}

static void tremolo_prepare(void * ctx, struct audio_frame_t * input)
{
	struct tremolo_pdata_t * pdat = (struct tremolo_pdata_t *)ctx;

	if((input->rate != pdat->__rate) || pdat->__setup)
	{
		int ntable = (int)rint(input->rate / pdat->frequency + 0.5);
		if(ntable > pdat->ntable)
		{
			pdat->ntable = ntable;
			pdat->table = xos_mem_realloc(pdat->table, sizeof(float) * pdat->ntable);
		}
		double offset = 1.0f - pdat->depth / 2.0f;
		for(int i = 0; i < pdat->ntable; i++)
		{
			double env = pdat->frequency * i / input->rate;
			env = sin(2 * M_PI * fmod(env + 0.25, 1.0));
			pdat->table[i] = env * (1 - fabs(offset)) + offset;
		}
		pdat->index = 0;
		pdat->__setup = 0;
		pdat->__rate = input->rate;
	}
}

static struct audio_frame_t * tremolo_process(void * ctx, struct audio_frame_t * input)
{
	struct tremolo_pdata_t * pdat = (struct tremolo_pdata_t *)ctx;
	float * pi = input->samples;

	for(int n = 0; n < input->frames; n++)
	{
		for(int c = 0; c < input->channel; c++)
		{
			pi[c] = pi[c] * pdat->table[pdat->index];
		}
		pi += input->channel;
		pdat->index++;
		if(pdat->index >= pdat->ntable)
			pdat->index = 0;
	}
	return input;
}

static void tremolo_destroy(void * ctx)
{
	struct tremolo_pdata_t * pdat = (struct tremolo_pdata_t *)ctx;

	if(pdat)
	{
		if(pdat->table)
			xos_mem_free(pdat->table);
		xos_mem_free(pdat);
	}
}

static struct audio_effect_t tremolo = {
	.name		= "tremolo",
	.create		= tremolo_create,
	.setup		= tremolo_setup,
	.prepare	= tremolo_prepare,
	.process	= tremolo_process,
	.destroy	= tremolo_destroy,
};

static void effect_tremolo_init(void)
{
	register_audio_effect(&tremolo);
}

static void effect_tremolo_exit(void)
{
	unregister_audio_effect(&tremolo);
}

core_initcall(effect_tremolo_init);
core_exitcall(effect_tremolo_exit);
