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
#include <kernel/audio/source.h>

struct audio_source_mixer_pdata_t {
	struct audio_mixer_t * mixer;
};

static int audio_source_mixer_seek(struct audio_source_t * s, int offset)
{
	return 0;
}

static int audio_source_mixer_tell(struct audio_source_t * s)
{
	return 0;
}

static int audio_source_mixer_length(struct audio_source_t * s)
{
	return 0;
}

static struct audio_frame_t * audio_source_mixer_read(struct audio_source_t * s)
{
	struct audio_source_mixer_pdata_t * pdat = (struct audio_source_mixer_pdata_t *)s->priv;
	return audio_mixer_read(pdat->mixer);
}

static int audio_source_mixer_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	struct audio_source_mixer_pdata_t * pdat = (struct audio_source_mixer_pdata_t *)s->priv;

	switch(shash(cmd))
	{
	case 0x9353e513: /* "audio-source-set-volume" */
		if(arg)
		{
			int * p = arg;
			audio_mixer_set_volume(pdat->mixer, p[0]);
			return 0;
		}
		break;

	case 0x62d51787: /* "audio-source-get-volume" */
		if(arg)
		{
			int * p = arg;
			p[0] = audio_mixer_get_volume(pdat->mixer);
			return 0;
		}
		break;

	default:
		break;
	}
	return -1;
}

static void audio_source_mixer_destroy(struct audio_source_t * s)
{
	struct audio_source_mixer_pdata_t * pdat = (struct audio_source_mixer_pdata_t *)s->priv;

	if(pdat)
		xos_mem_free(pdat);
}

struct audio_source_t * audio_source_alloc_from_mixer(struct audio_mixer_t * mixer)
{
	if(!mixer)
		return NULL;

	struct audio_source_mixer_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_source_mixer_pdata_t));
	if(!pdat)
		return NULL;

	pdat->mixer = mixer;

	struct audio_source_t * s = audio_source_alloc();
	if(!s)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	s->seek = audio_source_mixer_seek;
	s->tell = audio_source_mixer_tell;
	s->length = audio_source_mixer_length;
	s->read = audio_source_mixer_read;
	s->ioctl = audio_source_mixer_ioctl;
	s->destroy = audio_source_mixer_destroy;
	s->priv = pdat;

	return s;
}
