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

#include <xstar.h>

struct audio_source_capture_pdata_t {
	struct audio_capture_t * capture;
	int rate;
	int channel;

	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static int audio_source_capture_seek(struct audio_source_t * s, int offset)
{
	return 0;
}

static int audio_source_capture_tell(struct audio_source_t * s)
{
	return 0;
}

static int audio_source_capture_length(struct audio_source_t * s)
{
	return 0;
}

static struct audio_frame_t * audio_source_capture_read(struct audio_source_t * s)
{
	struct audio_source_capture_pdata_t * pdat = (struct audio_source_capture_pdata_t *)s->priv;
	float * po = pdat->samples;
	int maxlen = pdat->nsample;
	int len = 0;
	int us = 0;

	do {
		int n = maxlen - len;
		int l = audio_capture_read(pdat->capture, &po[len], maxlen - len);
		if(xstar_feature_thread())
		{
			if(l < n)
			{
				us += 100;
				if(us > 5000)
					us = 5000;
				xos_thread_usleep(us);
			}
			else
				us = 0;
		}
		len += l;
	} while(len < maxlen);

	pdat->output.rate = pdat->rate;
	pdat->output.channel = pdat->channel;
	pdat->output.frames = maxlen / pdat->channel;
	pdat->output.samples = pdat->samples;
	return &pdat->output;
}

static int audio_source_capture_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	struct audio_source_capture_pdata_t * pdat = (struct audio_source_capture_pdata_t *)s->priv;

	switch(shash(cmd))
	{
	case 0x9353e513: /* "audio-source-set-volume" */
		if(arg)
		{
			int * p = arg;
			audio_capture_set_volume(pdat->capture, p[0]);
			return 0;
		}
		break;

	case 0x62d51787: /* "audio-source-get-volume" */
		if(arg)
		{
			int * p = arg;
			p[0] = audio_capture_get_volume(pdat->capture);
			return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static void audio_source_capture_destroy(struct audio_source_t * s)
{
	struct audio_source_capture_pdata_t * pdat = (struct audio_source_capture_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->capture)
			audio_capture_stop(pdat->capture);
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

struct audio_source_t * audio_source_alloc_from_capture(const char * name, int rate, int channel)
{
	struct audio_capture_t * capture = search_audio_capture(name);
	if(!capture)
		capture = search_first_audio_capture();
	if(!audio_capture_start(capture, rate, channel))
		return NULL;

	struct audio_source_capture_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_source_capture_pdata_t));
	if(!pdat)
		return NULL;

	pdat->capture = capture;
	pdat->rate = rate;
	pdat->channel = channel;

	pdat->nsample = pdat->rate * pdat->channel / 200;
	pdat->samples = xos_mem_malloc(pdat->nsample * sizeof(float));
	if(!pdat->samples)
	{
		audio_capture_stop(pdat->capture);
		xos_mem_free(pdat);
		return NULL;
	}

	struct audio_source_t * s = audio_source_alloc();
	if(!s)
	{
		audio_capture_stop(pdat->capture);
		xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
		return NULL;
	}
	s->seek = audio_source_capture_seek;
	s->tell = audio_source_capture_tell;
	s->length = audio_source_capture_length;
	s->read = audio_source_capture_read;
	s->ioctl = audio_source_capture_ioctl;
	s->destroy = audio_source_capture_destroy;
	s->priv = pdat;

	return s;
}
