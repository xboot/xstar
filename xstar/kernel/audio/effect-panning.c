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
 * Panning - Remix channels with coefficients.
 *
 * Example:
 *   "panning": {
 *       "weight" : [
 * 	         1.0, 0.0,
 *           0.0, 1.0
 * 	     ]
 *   }
 */

#define MAX_PANNING_CHANNEL	(32)

struct panning_pdata_t {
	float weight[MAX_PANNING_CHANNEL][MAX_PANNING_CHANNEL];
};

static void * panning_create(void)
{
	struct panning_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct panning_pdata_t));
	if(!pdat)
		return NULL;

	for(int u = 0; u < MAX_PANNING_CHANNEL; u++)
	{
		for(int v = 0; v < MAX_PANNING_CHANNEL; v++)
		{
			if(u == v)
				pdat->weight[u][v] = 1.0;
			else
				pdat->weight[u][v] = 0.0;
		}
	}
	return pdat;
}

static void panning_setup(void * ctx, struct json_value_t * v)
{
	struct panning_pdata_t * pdat = (struct panning_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0x24d3ea4d: /* "weight" */
				if(o && (o->type == JSON_ARRAY))
				{
					int c = sqrti(o->u.array.length);
					if(c <= MAX_PANNING_CHANNEL)
					{
						for(int u = 0, idx = 0; u < c; u++)
						{
							for(int v = 0; v < c; v++)
							{
								struct json_value_t * e = o->u.array.values[idx++];
								if(e && (e->type == JSON_DOUBLE))
									pdat->weight[u][v] = (float)e->u.dbl;
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	}
}

static void panning_prepare(void * ctx, struct audio_frame_t * input)
{
}

static struct audio_frame_t * panning_process(void * ctx, struct audio_frame_t * input)
{
	struct panning_pdata_t * pdat = (struct panning_pdata_t *)ctx;
	float * pi = input->samples;
	float tmp[input->channel];
	int c = XMIN(input->channel, MAX_PANNING_CHANNEL);

	for(int n = 0; n < input->frames; n++)
	{
		for(int u = 0; u < c; u++)
		{
			tmp[u] = 0;
			for(int v = 0; v < c; v++)
				tmp[u] += pi[v] * pdat->weight[u][v];
		}
		for(int i = 0; i < input->channel; i++)
			pi[i] = tmp[i];
		pi += input->channel;
	}
	return input;
}

static void panning_destroy(void * ctx)
{
	struct panning_pdata_t * pdat = (struct panning_pdata_t *)ctx;

	if(pdat)
		xos_mem_free(pdat);
}

static struct audio_effect_t panning = {
	.name		= "panning",
	.create		= panning_create,
	.setup		= panning_setup,
	.prepare	= panning_prepare,
	.process	= panning_process,
	.destroy	= panning_destroy,
};

static void effect_panning_init(void)
{
	register_audio_effect(&panning);
}

static void effect_panning_exit(void)
{
	unregister_audio_effect(&panning);
}

core_initcall(effect_panning_init);
core_exitcall(effect_panning_exit);
