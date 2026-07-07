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

struct audio_source_tone_pdata_t {
	int rate;
	int channel;
	int loop;
	float factor;
	float * waveform;
	int wlen;
	int index;

	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static int audio_source_tone_seek(struct audio_source_t * s, int offset)
{
	struct audio_source_tone_pdata_t * pdat = (struct audio_source_tone_pdata_t *)s->priv;

	if(!pdat->loop)
	{
		if(offset < 0)
			pdat->index = XCLAMP(pdat->wlen + offset, 0, pdat->wlen - 1);
		else
			pdat->index = XCLAMP(offset, 0, pdat->wlen - 1);
		return pdat->index;
	}
	return 0;
}

static int audio_source_tone_tell(struct audio_source_t * s)
{
	struct audio_source_tone_pdata_t * pdat = (struct audio_source_tone_pdata_t *)s->priv;

	if(!pdat->loop)
		return pdat->index;
	return 0;
}

static int audio_source_tone_length(struct audio_source_t * s)
{
	struct audio_source_tone_pdata_t * pdat = (struct audio_source_tone_pdata_t *)s->priv;

	if(!pdat->loop)
		return pdat->wlen;
	return 0;
}

static struct audio_frame_t * audio_source_tone_read(struct audio_source_t * s)
{
	struct audio_source_tone_pdata_t * pdat = (struct audio_source_tone_pdata_t *)s->priv;
	float * po = pdat->samples;
	int frames = 0;

	if(pdat->loop)
	{
		for(int i = 0; i < (int)(pdat->nsample / pdat->channel); i++)
		{
			float v = pdat->waveform[pdat->index++] * pdat->factor;
			for(int c = 0; c < pdat->channel; c++)
				po[c] = v;
			po += pdat->channel;
			frames++;
			if(pdat->index >= pdat->wlen)
				pdat->index = 0;
		}
	}
	else
	{
		for(int i = 0; (pdat->index < pdat->wlen) && (i < (int)(pdat->nsample / pdat->channel)); i++)
		{
			float v = pdat->waveform[pdat->index++] * pdat->factor;
			for(int c = 0; c < pdat->channel; c++)
				po[c] = v;
			po += pdat->channel;
			frames++;
		}
	}
	pdat->output.rate = pdat->rate;
	pdat->output.channel = pdat->channel;
	pdat->output.frames = frames;
	pdat->output.samples = pdat->samples;
	return &pdat->output;
}

static int audio_source_tone_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	struct audio_source_tone_pdata_t * pdat = (struct audio_source_tone_pdata_t *)s->priv;

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

static void audio_source_tone_destroy(struct audio_source_t * s)
{
	struct audio_source_tone_pdata_t * pdat = (struct audio_source_tone_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		if(pdat->waveform)
			xos_mem_free(pdat->waveform);
		xos_mem_free(pdat);
	}
}

struct audio_source_t * audio_source_alloc_tone(char * type, int rate, int channel, int frequency, int millisecond)
{
	float amp = 1.0f;

	struct audio_source_tone_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_source_tone_pdata_t));
	if(!pdat)
		return NULL;

	pdat->rate = XMAX(rate, frequency * 2);
	pdat->channel = XMAX(channel, 1);
	pdat->factor = 1.0;
	if(millisecond <= 0)
	{
		pdat->wlen = (int)rint(pdat->rate / frequency + 0.5);
		pdat->loop = 1;
	}
	else
	{
		pdat->wlen = millisecond * pdat->rate / 1000;
		pdat->loop = 0;
	}
	pdat->waveform = xos_mem_malloc(sizeof(float) * pdat->wlen);
	if(!pdat->waveform)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	switch(shash(type))
	{
	case 0x7c9dec54: /* "sine" */
		{
			float step = 2 * M_PI * (float)frequency / (float)pdat->rate;
			for(int i = 0; i < pdat->wlen; i++)
				pdat->waveform[i] = amp * sinf(step * (float)i);
		}
		break;

	case 0x1c5eea16: /* "square" */
		{
			float step = 2 * M_PI * (float)frequency / (float)pdat->rate;
			for(int i = 0; i < pdat->wlen; i++)
			{
				float y = sinf(step * (float)i);
				if(y > 0)
					pdat->waveform[i] = amp;
				else if(y < 0)
					pdat->waveform[i] = -amp;
				else
					pdat->waveform[i] = 0;
			}
		}
		break;

	case 0xccfdd6fb: /* "triangle" */
		{
			float step = 2 * M_PI * (float)frequency / (float)pdat->rate;
			for(int i = 0; i < pdat->wlen; i++)
			{
				float x = step * (float)i - M_PI_2;
				pdat->waveform[i] = 2 * fabsf(-(2 * amp / M_PI) * atanf(cosf(x / 2) / sinf(x / 2))) - amp;
			}
		}
		break;

	case 0xe61eb31e: /* "sawtooth" */
		{
			float step = 2 * M_PI * (float)frequency / (float)pdat->rate;
			for(int i = 0; i < pdat->wlen; i++)
			{
				float x = (step * (float)i - M_PI) / 2;
				pdat->waveform[i] = -(2 * amp / M_PI) * atanf(cosf(x) / sinf(x));
			}
		}
		break;

	default:
		break;
	}
	pdat->index = 0;
	pdat->nsample = pdat->rate * pdat->channel / 200;
	pdat->samples = xos_mem_malloc(pdat->nsample * sizeof(float));
	if(!pdat->samples)
	{
		xos_mem_free(pdat->waveform);
		xos_mem_free(pdat);
		return NULL;
	}

	struct audio_source_t * s = audio_source_alloc();
	if(!s)
	{
		xos_mem_free(pdat->samples);
		xos_mem_free(pdat->waveform);
		xos_mem_free(pdat);
		return NULL;
	}
	s->seek = audio_source_tone_seek;
	s->tell = audio_source_tone_tell;
	s->length = audio_source_tone_length;
	s->read = audio_source_tone_read;
	s->ioctl = audio_source_tone_ioctl;
	s->destroy = audio_source_tone_destroy;
	s->priv = pdat;

	return s;
}
