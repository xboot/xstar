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

#define FFT_LOG2		(8)
#define FFT_SIZE		(1 << FFT_LOG2)
#define FFT_HALF		(1 << (FFT_LOG2 - 1))

struct audio_sink_spectrum_pdata_t {
	struct fft_t * fft;
	struct complex_t out[FFT_SIZE];
	float table[FFT_SIZE];
	float temp[FFT_SIZE];
	int rate;
	int index;
	int period;
	int elapsed;
};

static void audio_sink_spectrum_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
	struct audio_sink_spectrum_pdata_t * pdat = (struct audio_sink_spectrum_pdata_t *)s->priv;

	pdat->elapsed += audio_frame_total_time(af);
	if(pdat->elapsed >= pdat->period)
	{
		float * pi = af->samples;
		for(int n = 0; (n < af->frames) && (pdat->index < FFT_SIZE); n++)
		{
			float tmp = 0;
			for(int c = 0; c < af->channel; c++)
				tmp += pi[c];
			pdat->temp[pdat->index] = (tmp / af->channel) * pdat->table[pdat->index];
			pdat->index++;
			pi += af->channel;
		}
		if(pdat->index >= FFT_SIZE)
		{
			pdat->rate = af->rate;
			pdat->index = 0;
			pdat->elapsed = 0;
			fft_process_forward(pdat->fft, pdat->out, pdat->temp);
			psub_publish("audiosink.spectrum", (void *)s);
		}
	}
}

static int audio_sink_spectrum_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
	struct audio_sink_spectrum_pdata_t * pdat = (struct audio_sink_spectrum_pdata_t *)s->priv;

	switch(shash(cmd))
	{
	case 0x4e7a3d5f: /* "audio-sink-get-rate" */
		if(arg)
		{
			*((int *)arg) = pdat->rate;
			return 0;
		}
		break;

	case 0xcdedbb78: /* "audio-sink-get-amplitude" */
		if(arg)
		{
			float * p = (float *)arg;
			p[0] = complex_abs(&pdat->out[0]) / FFT_SIZE;
			for(int i = 1; i < FFT_HALF; i++)
				p[i] = complex_abs(&pdat->out[i]) * 2 / FFT_SIZE;
			p[FFT_HALF] = complex_abs(&pdat->out[FFT_HALF]) / FFT_SIZE;
			for(int i = FFT_HALF + 1; i < FFT_SIZE; i++)
				p[i] = complex_abs(&pdat->out[i]) * 2 / FFT_SIZE;
			return 0;
		}
		break;

	case 0x1da14044: /* "audio-sink-get-phase" */
		break;

	default:
		break;
	}
	return -1;
}

static void audio_sink_spectrum_destroy(struct audio_sink_t * s)
{
	struct audio_sink_spectrum_pdata_t * pdat = (struct audio_sink_spectrum_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->fft)
			fft_free(pdat->fft);
		xos_mem_free(pdat);
	}
}

struct audio_sink_t * audio_sink_alloc_spectrum(int period)
{
	struct audio_sink_spectrum_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_sink_spectrum_pdata_t));
	if(!pdat)
		return NULL;
	xos_memset(pdat, 0, sizeof(struct audio_sink_spectrum_pdata_t));

	pdat->fft = fft_new(FFT_LOG2);
	pdat->rate = 0;
	pdat->index = 0;
	pdat->period = (period > 0) ? period : 20;
	pdat->elapsed = 0;
	winfunc_hanning(&pdat->table[0], FFT_SIZE);

	struct audio_sink_t * s = audio_sink_alloc();
	if(!s)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	s->write = audio_sink_spectrum_write;
	s->ioctl = audio_sink_spectrum_ioctl;
	s->destroy = audio_sink_spectrum_destroy;
	s->priv = pdat;

	return s;
}
