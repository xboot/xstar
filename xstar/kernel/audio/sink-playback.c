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

struct audio_sink_playback_pdata_t {
	struct audio_playback_t * playback;
	int rate;
	int channel;

	struct audio_filter_t * filter;
};

static void audio_sink_playback_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
	struct audio_sink_playback_pdata_t * pdat = (struct audio_sink_playback_pdata_t *)s->priv;
	struct audio_frame_t * output;

	if(af->rate == pdat->rate)
		audio_filter_bypass(pdat->filter, 0, 1);
	else
		audio_filter_bypass(pdat->filter, 0, 0);
	if(af->channel == pdat->channel)
		audio_filter_bypass(pdat->filter, 1, 1);
	else
		audio_filter_bypass(pdat->filter, 1, 0);

	output = audio_filter_process(pdat->filter, af);
	if(output)
	{
		int maxlen = output->frames * output->channel;
		if(maxlen > 0)
		{
			int len = 0;
			int us = 0;
			do {
				int n = maxlen - len;
				int l = audio_playback_write(pdat->playback, &output->samples[len], n);
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
		}
	}
}

static int audio_sink_playback_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
	struct audio_sink_playback_pdata_t * pdat = (struct audio_sink_playback_pdata_t *)s->priv;

	switch(shash(cmd))
	{
	case 0x10cbc7b7: /* "audio-sink-set-volume" */
		if(arg)
		{
			int * p = arg;
			audio_playback_set_volume(pdat->playback, p[0]);
			return 0;
		}
		break;

	case 0xe04cfa2b: /* "audio-sink-get-volume" */
		if(arg)
		{
			int * p = arg;
			p[0] = audio_playback_get_volume(pdat->playback);
			return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static void audio_sink_playback_destroy(struct audio_sink_t * s)
{
	struct audio_sink_playback_pdata_t * pdat = (struct audio_sink_playback_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->playback)
			audio_playback_stop(pdat->playback);
		if(pdat->filter)
			audio_filter_free(pdat->filter);
		xos_mem_free(pdat);
	}
}

struct audio_sink_t * audio_sink_alloc_from_playback(const char * name, int rate, int channel)
{
	struct audio_playback_t * playback = search_audio_playback(name);
	if(!playback)
		playback = search_first_audio_playback();
	if(!audio_playback_start(playback, rate, channel))
		return NULL;

	struct audio_sink_playback_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_sink_playback_pdata_t));
	if(!pdat)
		return NULL;

	pdat->playback = playback;
	pdat->rate = rate;
	pdat->channel = channel;

	char json[256];
	int length = xos_sprintf(json, "{\"resample\":{\"rate\":%d},\"reshape\":{\"channel\":%d}}", pdat->rate, pdat->channel);
	pdat->filter = audio_filter_alloc(json, length);
	if(!pdat->filter)
	{
		audio_playback_stop(pdat->playback);
		xos_mem_free(pdat);
		return NULL;
	}

	struct audio_sink_t * s = audio_sink_alloc();
	if(!s)
	{
		audio_playback_stop(pdat->playback);
		audio_filter_free(pdat->filter);
		xos_mem_free(pdat);
		return NULL;
	}
	s->write = audio_sink_playback_write;
	s->ioctl = audio_sink_playback_ioctl;
	s->destroy = audio_sink_playback_destroy;
	s->priv = pdat;

	return s;
}
