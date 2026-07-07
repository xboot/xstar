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

#include <kernel/core/psub.h>
#include <kernel/audio/sink.h>

struct audio_sink_amplitude_pdata_t {
	float amplitude;
	int period;
	int elapsed;
};

static void audio_sink_amplitude_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
	struct audio_sink_amplitude_pdata_t * pdat = (struct audio_sink_amplitude_pdata_t *)s->priv;
	float * po = af->samples;

	for(int n = 0; n < af->frames; n++)
	{
		float v = 0;
		for(int c = 0; c < af->channel; c++)
			v += po[c];
		v = XCLAMP(fabsf(v / af->channel), 0.0f, 1.0f);
		if(pdat->amplitude < v)
			pdat->amplitude = v;
		po += af->channel;
	}

	pdat->elapsed += audio_frame_total_time(af);
	if(pdat->elapsed >= pdat->period)
	{
		psub_publish("audiosink.amplitude", (void *)s);
		pdat->amplitude = 0.0f;
		pdat->elapsed = 0;
	}
}

static int audio_sink_amplitude_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
	struct audio_sink_amplitude_pdata_t * pdat = (struct audio_sink_amplitude_pdata_t *)s->priv;

	switch(shash(cmd))
	{
	case 0x10cbc7b7: /* "audio-sink-set-volume" */
		break;

	case 0xe04cfa2b: /* "audio-sink-get-volume" */
		break;

	case 0xcdedbb78: /* "audio-sink-get-amplitude" */
		if(arg)
		{
			*((float *)arg) = pdat->amplitude;
			return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static void audio_sink_amplitude_destroy(struct audio_sink_t * s)
{
	struct audio_sink_amplitude_pdata_t * pdat = (struct audio_sink_amplitude_pdata_t *)s->priv;

	if(pdat)
		xos_mem_free(pdat);
}

struct audio_sink_t * audio_sink_alloc_amplitude(int period)
{
	struct audio_sink_amplitude_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_sink_amplitude_pdata_t));
	if(!pdat)
		return NULL;

	pdat->amplitude = 0.0f;
	pdat->period = (period > 0) ? period : 20;
	pdat->elapsed = 0;

	struct audio_sink_t * s = audio_sink_alloc();
	if(!s)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	s->write = audio_sink_amplitude_write;
	s->ioctl = audio_sink_amplitude_ioctl;
	s->destroy = audio_sink_amplitude_destroy;
	s->priv = pdat;

	return s;
}
