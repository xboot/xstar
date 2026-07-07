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

struct wav_header_t {
	uint8_t riff[4];
	uint32_t riffsz;
	uint8_t wave[4];
	uint8_t fmt[4];
	uint32_t fmtsz;
	uint16_t fmttag;
	uint16_t channel;
	uint32_t samplerate;
	uint32_t byterate;
	uint16_t align;
	uint16_t bps;
	uint8_t data[4];
	uint32_t datasz;
};

struct audio_source_wav_pdata_t {
	struct xfs_file_t * file;
	int rate;
	int channel;
	int depth;
	float factor;
	int frames;
	int offset;
	char * buffer;
	int nbuffer;

	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static int audio_source_wav_seek(struct audio_source_t * s, int offset)
{
	struct audio_source_wav_pdata_t * pdat = (struct audio_source_wav_pdata_t *)s->priv;

	if(offset < 0)
		pdat->offset = XCLAMP(pdat->frames + offset, 0, pdat->frames - 1);
	else
		pdat->offset = XCLAMP(offset, 0, pdat->frames - 1);
	return pdat->offset;
}

static int audio_source_wav_tell(struct audio_source_t * s)
{
	struct audio_source_wav_pdata_t * pdat = (struct audio_source_wav_pdata_t *)s->priv;

	return pdat->offset;
}

static int audio_source_wav_length(struct audio_source_t * s)
{
	struct audio_source_wav_pdata_t * pdat = (struct audio_source_wav_pdata_t *)s->priv;

	return pdat->frames;
}

static struct audio_frame_t * audio_source_wav_read(struct audio_source_t * s)
{
	struct audio_source_wav_pdata_t * pdat = (struct audio_source_wav_pdata_t *)s->priv;
	float * po = pdat->samples;
	int bytes = pdat->channel * (pdat->depth >> 3);
	int frames = 0;
	int64_t n;

	xfs_seek(pdat->file, sizeof(struct wav_header_t) + pdat->offset * bytes);
	n = xfs_read(pdat->file, (void *)pdat->buffer, pdat->nbuffer);

	switch(pdat->depth)
	{
	case 8:
		{
			float factor = pdat->factor * (1.0 / 0x80);
			int8_t * pi = (int8_t *)pdat->buffer;
			for(int i = 0; i < (int)(n / bytes); i++)
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
			int16_t * pi = (int16_t *)pdat->buffer;
			for(int i = 0; i < (int)(n / bytes); i++)
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

static int audio_source_wav_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	struct audio_source_wav_pdata_t * pdat = (struct audio_source_wav_pdata_t *)s->priv;

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

static void audio_source_wav_destroy(struct audio_source_t * s)
{
	struct audio_source_wav_pdata_t * pdat = (struct audio_source_wav_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->file)
			xfs_close(pdat->file);
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		if(pdat->buffer)
			xos_mem_free(pdat->buffer);
		xos_mem_free(pdat);
	}
}

struct audio_source_t * audio_source_alloc_from_xfs_wav(struct xfs_context_t * ctx, const char * filename)
{
	struct wav_header_t header;
	struct xfs_file_t * file;

	if(!(file = xfs_open_read(ctx, filename)))
		return NULL;

	if(xfs_read(file, &header, sizeof(struct wav_header_t)) != sizeof(struct wav_header_t))
	{
		xfs_close(file);
		return NULL;
	}

	header.riffsz = le32_to_cpu(header.riffsz);
	header.fmtsz = le32_to_cpu(header.fmtsz);
	header.fmttag = le16_to_cpu(header.fmttag);
	header.channel = le16_to_cpu(header.channel);
	header.samplerate = le32_to_cpu(header.samplerate);
	header.byterate = le32_to_cpu(header.byterate);
	header.align = le16_to_cpu(header.align);
	header.bps = le16_to_cpu(header.bps);
	header.datasz = le32_to_cpu(header.datasz);

	if( (xos_memcmp(header.riff, "RIFF", 4) != 0) ||
		(xos_memcmp(header.wave, "WAVE", 4) != 0) ||
		(xos_memcmp(header.fmt,  "fmt ", 4) != 0) ||
		(xos_memcmp(header.data, "data", 4) != 0) ||
		(header.fmttag != 1) || (header.datasz < header.align) )
	{
		xfs_close(file);
		return NULL;
	}
	if(((header.channel != 1) && (header.channel != 2)) || ((header.bps != 8) && (header.bps != 16)))
	{
		xfs_close(file);
		return NULL;
	}

	struct audio_source_wav_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_source_wav_pdata_t));
	if(!pdat)
	{
		xfs_close(file);
		return NULL;
	}

	pdat->file = file;
	pdat->rate = header.samplerate;
	pdat->channel = header.channel;
	pdat->depth = header.bps;
	pdat->factor = 1.0;
	pdat->frames = header.datasz / header.align;
	pdat->offset = 0;

	pdat->nsample = pdat->rate * pdat->channel / 200;
	pdat->samples = xos_mem_malloc(pdat->nsample * sizeof(float));
	if(!pdat->samples)
	{
		xfs_close(pdat->file);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->nbuffer = pdat->nsample * (pdat->depth >> 3);
	pdat->buffer = xos_mem_malloc(pdat->nbuffer);
	if(!pdat->buffer)
	{
		xfs_close(pdat->file);
		xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
		return NULL;
	}

	struct audio_source_t * s = audio_source_alloc();
	if(!s)
	{
		xfs_close(pdat->file);
		xos_mem_free(pdat->samples);
		xos_mem_free(pdat->buffer);
		xos_mem_free(pdat);
		return NULL;
	}
	s->seek = audio_source_wav_seek;
	s->tell = audio_source_wav_tell;
	s->length = audio_source_wav_length;
	s->read = audio_source_wav_read;
	s->ioctl = audio_source_wav_ioctl;
	s->destroy = audio_source_wav_destroy;
	s->priv = pdat;

	return s;
}
