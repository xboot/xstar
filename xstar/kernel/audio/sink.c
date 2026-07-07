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

#include <kernel/audio/sink.h>

struct audio_sink_t * audio_sink_alloc(void)
{
	struct audio_sink_t * s = xos_mem_malloc(sizeof(struct audio_sink_t));

	if(s)
		xos_memset(s, 0, sizeof(struct audio_sink_t));
	return s;
}

void audio_sink_free(struct audio_sink_t * s)
{
	if(s)
	{
		if(s->destroy)
			s->destroy(s);
		if(s->filter)
			audio_filter_free(s->filter);
		xos_mem_free(s);
	}
}

struct audio_filter_t * audio_sink_filter_apply(struct audio_sink_t * s, const char * json, int len)
{
	if(s)
	{
		if(s->filter)
			audio_filter_free(s->filter);
		s->filter = audio_filter_alloc(json, len);
		return s->filter;
	}
	return NULL;
}

void audio_sink_filter_bypass(struct audio_sink_t * s, int id, int bypass)
{
	if(s && s->filter)
		audio_filter_bypass(s->filter, id, bypass);
}

void audio_sink_filter_setup(struct audio_sink_t * s, int id, const char * json, int len)
{
	if(s && s->filter)
		audio_filter_setup(s->filter, id, json, len);
}

void audio_sink_filter_bypass_by_name(struct audio_sink_t * s, const char * name, int bypass)
{
	if(s && s->filter)
		audio_filter_bypass_by_name(s->filter, name, bypass);
}

void audio_sink_filter_setup_by_name(struct audio_sink_t * s, const char * name, const char * json, int len)
{
	if(s && s->filter)
		audio_filter_setup_by_name(s->filter, name, json, len);
}

void audio_sink_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
	if(s && s->write && af)
		s->write(s, audio_filter_process(s->filter, af));
}

int audio_sink_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
	if(s && s->ioctl)
		return s->ioctl(s, cmd, arg);
	return -1;
}

int audio_sink_get_volume(struct audio_sink_t * s)
{
	int vol;

	if(audio_sink_ioctl(s, "audio-sink-get-volume", &vol) >= 0)
		return vol;
	return 0;
}

void audio_sink_set_volume(struct audio_sink_t * s, int vol)
{
	if(vol < 0)
		vol = 0;
	else if(vol > 1000)
		vol = 1000;
	audio_sink_ioctl(s, "audio-sink-set-volume", &vol);
}
