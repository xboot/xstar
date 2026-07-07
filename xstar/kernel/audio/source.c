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

struct audio_source_t * audio_source_alloc(void)
{
	struct audio_source_t * s = xos_mem_malloc(sizeof(struct audio_source_t));

	if(s)
		xos_memset(s, 0, sizeof(struct audio_source_t));
	return s;
}

void audio_source_free(struct audio_source_t * s)
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

struct audio_filter_t * audio_source_filter_apply(struct audio_source_t * s, const char * json, int len)
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

void audio_source_filter_bypass(struct audio_source_t * s, int id, int bypass)
{
	if(s && s->filter)
		audio_filter_bypass(s->filter, id, bypass);
}

void audio_source_filter_setup(struct audio_source_t * s, int id, const char * json, int len)
{
	if(s && s->filter)
		audio_filter_setup(s->filter, id, json, len);
}

void audio_source_filter_bypass_by_name(struct audio_source_t * s, const char * name, int bypass)
{
	if(s && s->filter)
		audio_filter_bypass_by_name(s->filter, name, bypass);
}

void audio_source_filter_setup_by_name(struct audio_source_t * s, const char * name, const char * json, int len)
{
	if(s && s->filter)
		audio_filter_setup_by_name(s->filter, name, json, len);
}

int audio_source_seek(struct audio_source_t * s, int offset)
{
	if(s && s->seek)
		return s->seek(s, offset);
	return 0;
}

int audio_source_tell(struct audio_source_t * s)
{
	if(s && s->tell)
		return s->tell(s);
	return 0;
}

int audio_source_length(struct audio_source_t * s)
{
	if(s && s->length)
		return s->length(s);
	return 0;
}

struct audio_frame_t * audio_source_read(struct audio_source_t * s)
{
	if(s && s->read)
		return audio_filter_process(s->filter, s->read(s));
	return NULL;
}

int audio_source_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	if(s && s->ioctl)
		return s->ioctl(s, cmd, arg);
	return -1;
}

int audio_source_get_volume(struct audio_source_t * s)
{
	int vol;

	if(audio_source_ioctl(s, "audio-source-get-volume", &vol) >= 0)
		return vol;
	return 0;
}

void audio_source_set_volume(struct audio_source_t * s, int vol)
{
	if(vol < 0)
		vol = 0;
	else if(vol > 1000)
		vol = 1000;
	audio_source_ioctl(s, "audio-source-set-volume", &vol);
}
