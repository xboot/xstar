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

#include <kernel/audio/mixer.h>

struct audio_sound_t {
	struct list_head_t entry;
	struct audio_source_t * source;
	struct audio_filter_t * filter;

	struct audio_frame_t * af;
	int pos;
};

struct audio_mixer_t * audio_mixer_alloc(int rate, int channel)
{
	struct audio_mixer_t * m = xos_mem_malloc(sizeof(struct audio_mixer_t));

	if(m)
	{
		xos_memset(m, 0, sizeof(struct audio_mixer_t));
		init_list_head(&m->list);
		xos_mutex_init(&m->lock);
		m->rate = XMAX(rate, 1000);
		m->channel = XMAX(channel, 1);
		m->mixed = 1.0f;
		m->factor = 1.0f;
		m->nsample = m->rate * m->channel / 200;
		m->samples = xos_mem_malloc(m->nsample * sizeof(float));
		if(!m->samples)
		{
			xos_mem_free(m);
			return NULL;
		}
		xos_memset(m->samples, 0, m->nsample * sizeof(float));
		m->output.rate = m->rate;
		m->output.channel = m->channel;
		m->output.frames = m->nsample / m->channel;
		m->output.samples = m->samples;
	}
	return m;
}

void audio_mixer_free(struct audio_mixer_t * m)
{
	if(m)
	{
		audio_mixer_clear(m);
		xos_mutex_exit(&m->lock);
		if(m->samples)
			xos_mem_free(m->samples);
		xos_mem_free(m);
	}
}

void audio_mixer_add(struct audio_mixer_t * m, struct audio_source_t * s)
{
	if(m && s)
	{
		struct audio_sound_t * pos, * n;
		list_for_each_entry_safe(pos, n, &m->list, entry)
		{
			if(pos->source == s)
				return;
		}
		struct audio_sound_t * snd = xos_mem_malloc(sizeof(struct audio_sound_t));
		if(snd)
		{
			char json[256];
			int length = xos_sprintf(json, "{\"resample\":{\"rate\":%d},\"reshape\":{\"channel\":%d}}", m->rate, m->channel);
			xos_memset(snd, 0, sizeof(struct audio_sound_t));
			init_list_head(&snd->entry);
			snd->source = s;
			snd->filter = audio_filter_alloc(json, length);
			snd->af = NULL;
			snd->pos = 0;
			xos_mutex_lock(&m->lock);
			list_add_tail(&snd->entry, &m->list);
			xos_mutex_unlock(&m->lock);
		}
	}
}

void audio_mixer_remove(struct audio_mixer_t * m, struct audio_source_t * s)
{
	if(m && s)
	{
		struct audio_sound_t * pos, * n;
		list_for_each_entry_safe(pos, n, &m->list, entry)
		{
			if(pos && (pos->source == s))
			{
				xos_mutex_lock(&m->lock);
				list_del(&pos->entry);
				xos_mutex_unlock(&m->lock);
				if(pos->filter)
					audio_filter_free(pos->filter);
				xos_mem_free(pos);
			}
		}
	}
}

void audio_mixer_clear(struct audio_mixer_t * m)
{
	if(m)
	{
		struct audio_sound_t * pos, * n;
		list_for_each_entry_safe(pos, n, &m->list, entry)
		{
			if(pos)
			{
				xos_mutex_lock(&m->lock);
				list_del(&pos->entry);
				xos_mutex_unlock(&m->lock);
				if(pos->filter)
					audio_filter_free(pos->filter);
				xos_mem_free(pos);
			}
		}
	}
}

static inline int audio_sound_fill(struct audio_sound_t * snd, float * samples, int nsample)
{
	int n;

	if(!snd->af || (snd->pos >= snd->af->frames * snd->af->channel))
	{
		snd->af = audio_filter_process(snd->filter, audio_source_read(snd->source));
		snd->pos = 0;
	}
	for(n = 0; (n < nsample) && (snd->pos < snd->af->frames * snd->af->channel); n++)
		samples[n] += snd->af->samples[snd->pos++];
	return n;
}

struct audio_frame_t * audio_mixer_read(struct audio_mixer_t * m)
{
	if(m)
	{
		struct audio_sound_t * pos;
		int maxlen = 0;
		xos_memset(m->samples, 0, m->nsample * sizeof(float));
		list_for_each_entry(pos, &m->list, entry)
		{
			if(pos)
			{
				int len = audio_sound_fill(pos, m->samples, m->nsample);
				if(len > maxlen)
					maxlen = len;
			}
		}
		if(maxlen > 0)
		{
			for(int i = 0; i < maxlen; i++)
			{
				float t = m->samples[i] * m->mixed;
				if(t > 1.0f)
				{
					m->mixed = 1.0f / t;
					t = 1.0f;
				}
				else if(t < -1.0f)
				{
					m->mixed = -1.0f / t;
					t = -1.0f;
				}
				if(m->mixed < 1.0f)
					m->mixed += (1.0f - m->mixed) / 32.0f;
				m->samples[i] = t * m->factor;
			}
		}
		m->output.rate = m->rate;
		m->output.channel = m->channel;
		m->output.frames = maxlen / m->channel;
		m->output.samples = m->samples;
		return &m->output;
	}
	return NULL;
}

int audio_mixer_get_volume(struct audio_mixer_t * m)
{
	if(m)
		return factor_to_volume(m->factor);
	return 0;
}

void audio_mixer_set_volume(struct audio_mixer_t * m, int vol)
{
	if(m)
		m->factor = volume_to_factor(XCLAMP(vol, 0, 1000));
}
