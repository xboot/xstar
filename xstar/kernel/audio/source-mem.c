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

#include <kernel/audio/source.h>

struct audio_source_mem_pdata_t {
	int rate;
	int channel;
	int depth;
	float factor;
	unsigned char * mem;
	int mlen;
	int offset;

	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static int audio_source_mem_seek(struct audio_source_t * s, int offset)
{
	struct audio_source_mem_pdata_t * pdat = (struct audio_source_mem_pdata_t *)s->priv;
	int bytes = pdat->channel * (pdat->depth >> 3);

	if(offset < 0)
		pdat->offset = XCLAMP(pdat->mlen / bytes + offset, 0, pdat->mlen / bytes - 1);
	else
		pdat->offset = XCLAMP(offset, 0, pdat->mlen / bytes - 1);
	return pdat->offset;
}

static int audio_source_mem_tell(struct audio_source_t * s)
{
	struct audio_source_mem_pdata_t * pdat = (struct audio_source_mem_pdata_t *)s->priv;

	return pdat->offset;
}

static int audio_source_mem_length(struct audio_source_t * s)
{
	struct audio_source_mem_pdata_t * pdat = (struct audio_source_mem_pdata_t *)s->priv;
	int bytes = pdat->channel * (pdat->depth >> 3);

	return pdat->mlen / bytes;
}

static struct audio_frame_t * audio_source_mem_read(struct audio_source_t * s)
{
	struct audio_source_mem_pdata_t * pdat = (struct audio_source_mem_pdata_t *)s->priv;
	float * po = pdat->samples;
	int bytes = pdat->channel * (pdat->depth >> 3);
	int frames = 0;

	switch(pdat->depth)
	{
	case 8:
		{
			float factor = pdat->factor * (1.0 / 0x80);
			int8_t * pi = (int8_t *)&pdat->mem[pdat->offset * bytes];
			for(int i = 0; (pdat->offset < (pdat->mlen / bytes)) && (i < (int)(pdat->nsample / pdat->channel)); i++)
			{
				for(int c = 0; c < pdat->channel; c++)
				{
					po[c] = (float)pi[c] * factor;
				}
				pi += pdat->channel;
				po += pdat->channel;
				frames++;
			}
			pdat->offset += frames;
		}
		break;

	case 16:
		{
			float factor = pdat->factor * (1.0 / 0x8000);
			int16_t * pi = (int16_t *)&pdat->mem[pdat->offset * bytes];
			for(int i = 0; (pdat->offset < (pdat->mlen / bytes)) && (i < (int)(pdat->nsample / pdat->channel)); i++)
			{
				for(int c = 0; c < pdat->channel; c++)
				{
					po[c] = (float)pi[c] * factor;
				}
				pi += pdat->channel;
				po += pdat->channel;
				frames++;
			}
			pdat->offset += frames;
		}
		break;

	case 32:
		{
			float factor = pdat->factor * (1.0 / 0x80000000);
			int32_t * pi = (int32_t *)&pdat->mem[pdat->offset * bytes];
			for(int i = 0; (pdat->offset < (pdat->mlen / bytes)) && (i < (int)(pdat->nsample / pdat->channel)); i++)
			{
				for(int c = 0; c < pdat->channel; c++)
				{
					po[c] = (float)pi[c] * factor;
				}
				pi += pdat->channel;
				po += pdat->channel;
				frames++;
			}
			pdat->offset += frames;
		}
		break;

	default:
		break;
	}
	pdat->output.rate = pdat->rate;
	pdat->output.channel = pdat->channel;
	pdat->output.frames = frames;
	pdat->output.samples = pdat->samples;
	return &pdat->output;
}

static int audio_source_mem_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	struct audio_source_mem_pdata_t * pdat = (struct audio_source_mem_pdata_t *)s->priv;

	switch(shash(cmd))
	{
	case 0x9353e513: /* "audio-source-set-volume" */
		if(arg)
		{
			int * p = arg;
			pdat->factor = volume_to_factor(p[0]);
			return 0;
		}
		break;

	case 0x62d51787: /* "audio-source-get-volume" */
		if(arg)
		{
			int * p = arg;
			p[0] = factor_to_volume(pdat->factor);
			return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static void audio_source_mem_destroy(struct audio_source_t * s)
{
	struct audio_source_mem_pdata_t * pdat = (struct audio_source_mem_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

struct audio_source_t * audio_source_alloc_from_memory(int rate, int channel, int depth, void * mem, int len)
{
	if(!mem || len < channel * (depth >> 3))
		return NULL;

	struct audio_source_mem_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_source_mem_pdata_t));
	if(!pdat)
		return NULL;

	pdat->rate = rate;
	pdat->channel = channel;
	pdat->depth = depth;
	pdat->factor = 1.0;
	pdat->mem = mem;
	pdat->mlen = len;
	pdat->offset = 0;

	pdat->nsample = pdat->rate * pdat->channel / 200;
	pdat->samples = xos_mem_malloc(pdat->nsample * sizeof(float));
	if(!pdat->samples)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	struct audio_source_t * s = audio_source_alloc();
	if(!s)
	{
		xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
		return NULL;
	}
	s->seek = audio_source_mem_seek;
	s->tell = audio_source_mem_tell;
	s->length = audio_source_mem_length;
	s->read = audio_source_mem_read;
	s->ioctl = audio_source_mem_ioctl;
	s->destroy = audio_source_mem_destroy;
	s->priv = pdat;

	return s;
}
