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

struct qoa_lms_t {
	int history[4];
	int weights[4];
};

struct qoa_desc_t {
	unsigned int channels;
	unsigned int samplerate;
	unsigned int samples;
	struct qoa_lms_t lms[8];
};

static inline uint64_t qoa_read_u64(const unsigned char * bytes, unsigned int * p)
{
	bytes += *p;
	*p += 8;
	return	((uint64_t)(bytes[0]) << 56) | ((uint64_t)(bytes[1]) << 48) |
			((uint64_t)(bytes[2]) << 40) | ((uint64_t)(bytes[3]) << 32) |
			((uint64_t)(bytes[4]) << 24) | ((uint64_t)(bytes[5]) << 16) |
			((uint64_t)(bytes[6]) <<  8) | ((uint64_t)(bytes[7]) <<  0);
}

static inline int qoa_clamp(int v, int min, int max)
{
	if(v < min)
		return min;
	if(v > max)
		return max;
	return v;
}

static inline int qoa_clamp_s16(int v)
{
	if((unsigned int)(v + 32768) > 65535)
	{
		if(v < -32768)
			return -32768;
		if(v > 32767)
			return 32767;
	}
	return v;
}

static const int qoa_dequant_tab[16][8] = {
	{   1,    -1,    3,    -3,    5,    -5,     7,     -7},
	{   5,    -5,   18,   -18,   32,   -32,    49,    -49},
	{  16,   -16,   53,   -53,   95,   -95,   147,   -147},
	{  34,   -34,  113,  -113,  203,  -203,   315,   -315},
	{  63,   -63,  210,  -210,  378,  -378,   588,   -588},
	{ 104,  -104,  345,  -345,  621,  -621,   966,   -966},
	{ 158,  -158,  528,  -528,  950,  -950,  1477,  -1477},
	{ 228,  -228,  760,  -760, 1368, -1368,  2128,  -2128},
	{ 316,  -316, 1053, -1053, 1895, -1895,  2947,  -2947},
	{ 422,  -422, 1405, -1405, 2529, -2529,  3934,  -3934},
	{ 548,  -548, 1828, -1828, 3290, -3290,  5117,  -5117},
	{ 696,  -696, 2320, -2320, 4176, -4176,  6496,  -6496},
	{ 868,  -868, 2893, -2893, 5207, -5207,  8099,  -8099},
	{1064, -1064, 3548, -3548, 6386, -6386,  9933,  -9933},
	{1286, -1286, 4288, -4288, 7718, -7718, 12005, -12005},
	{1536, -1536, 5120, -5120, 9216, -9216, 14336, -14336},
};

static int qoa_lms_predict(struct qoa_lms_t * lms)
{
	int prediction = 0;
	for(int i = 0; i < 4; i++)
		prediction += lms->weights[i] * lms->history[i];
	return prediction >> 13;
}

static void qoa_lms_update(struct qoa_lms_t * lms, int sample, int residual)
{
	int delta = residual >> 4;
	for(int i = 0; i < 4; i++)
		lms->weights[i] += lms->history[i] < 0 ? -delta : delta;
	for(int i = 0; i < 4 - 1; i++)
		lms->history[i] = lms->history[i + 1];
	lms->history[4 - 1] = sample;
}

static unsigned int qoa_max_frame_size(struct qoa_desc_t * info)
{
	return (8 + 4 * 4 * info->channels + 8 * 256 * info->channels);
}

static unsigned int qoa_decode_header(const unsigned char * bytes, int size, struct qoa_desc_t * info)
{
	unsigned int p = 0;
	if(size < 16)
		return 0;

	uint64_t file_header = qoa_read_u64(bytes, &p);
	if((file_header >> 32) != 0x716f6166)
		return 0;

	info->samples = file_header & 0xffffffff;
	if(!info->samples)
		return 0;

	uint64_t frame_header = qoa_read_u64(bytes, &p);
	info->channels = (frame_header >> 56) & 0x0000ff;
	info->samplerate = (frame_header >> 32) & 0xffffff;

	if(info->channels == 0 || info->samples == 0 || info->samplerate == 0)
		return 0;
	return 8;
}

static unsigned int qoa_decode_frame(const unsigned char * bytes, unsigned int size, struct qoa_desc_t * info, short * sample_data, unsigned int * frame_len)
{
	unsigned int p = 0;
	*frame_len = 0;

	if(size < 8 + 4 * 4 * info->channels)
		return 0;

	uint64_t frame_header = qoa_read_u64(bytes, &p);
	unsigned int channels = (frame_header >> 56) & 0x0000ff;
	unsigned int samplerate = (frame_header >> 32) & 0xffffff;
	unsigned int samples = (frame_header >> 16) & 0x00ffff;
	unsigned int frame_size = (frame_header) & 0x00ffff;

	unsigned int data_size = frame_size - 8 - 4 * 4 * channels;
	unsigned int num_slices = data_size / 8;
	unsigned int max_total_samples = num_slices * 20;

	if(channels != info->channels || samplerate != info->samplerate || frame_size > size || samples * channels > max_total_samples)
		return 0;

	for(unsigned int c = 0; c < channels; c++)
	{
		uint64_t history = qoa_read_u64(bytes, &p);
		uint64_t weights = qoa_read_u64(bytes, &p);
		for(int i = 0; i < 4; i++)
		{
			info->lms[c].history[i] = ((signed short)(history >> 48));
			history <<= 16;
			info->lms[c].weights[i] = ((signed short)(weights >> 48));
			weights <<= 16;
		}
	}

	for(unsigned int sample_index = 0; sample_index < samples; sample_index += 20)
	{
		for(unsigned int c = 0; c < channels; c++)
		{
			uint64_t slice = qoa_read_u64(bytes, &p);

			int scalefactor = (slice >> 60) & 0xf;
			slice <<= 4;

			int slice_start = sample_index * channels + c;
			int slice_end = qoa_clamp(sample_index + 20, 0, samples) * channels + c;

			for(int si = slice_start; si < slice_end; si += channels)
			{
				int predicted = qoa_lms_predict(&info->lms[c]);
				int quantized = (slice >> 61) & 0x7;
				int dequantized = qoa_dequant_tab[scalefactor][quantized];
				int reconstructed = qoa_clamp_s16(predicted + dequantized);

				sample_data[si] = reconstructed;
				slice <<= 3;

				qoa_lms_update(&info->lms[c], reconstructed, dequantized);
			}
		}
	}
	*frame_len = samples;
	return p;
}

struct audio_source_qoa_pdata_t {
	struct qoa_desc_t info;
	struct xfs_file_t * file;
	float factor;
	int first_frame_offset;
	char * buffer;
	int nbuffer;
	short * sample_data;
	int sample_data_size;
	unsigned int sample_data_len;
	unsigned int sample_data_pos;
	unsigned int sample_pos;

	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static int audio_source_qoa_seek(struct audio_source_t * s, int offset)
{
	struct audio_source_qoa_pdata_t * pdat = (struct audio_source_qoa_pdata_t *)s->priv;
	int maxframes = pdat->info.samples / (256 * 20);
	int frame;
	if(offset < 0)
		frame = XCLAMP(maxframes + offset, 0, maxframes - 1);
	else
		frame = XCLAMP(offset, 0, maxframes - 1);
	pdat->sample_data_pos = 0;
	pdat->sample_data_len = 0;
	pdat->sample_pos = frame * (256 * 20);
	xfs_seek(pdat->file, pdat->first_frame_offset + frame * qoa_max_frame_size(&pdat->info));
	return pdat->sample_pos / (256 * 20);
}

static int audio_source_qoa_tell(struct audio_source_t * s)
{
	struct audio_source_qoa_pdata_t * pdat = (struct audio_source_qoa_pdata_t *)s->priv;
	return pdat->sample_pos / (256 * 20);
}

static int audio_source_qoa_length(struct audio_source_t * s)
{
	struct audio_source_qoa_pdata_t * pdat = (struct audio_source_qoa_pdata_t *)s->priv;
	return pdat->info.samples / (256 * 20);
}

static inline unsigned int decode_frame(struct audio_source_qoa_pdata_t * pdat)
{
	unsigned int len = xfs_read(pdat->file, (void *)pdat->buffer, qoa_max_frame_size(&pdat->info));
	unsigned int frame_len;
	qoa_decode_frame((const unsigned char *)pdat->buffer, len, &pdat->info, pdat->sample_data, &frame_len);
	pdat->sample_data_len = frame_len;
	pdat->sample_data_pos = 0;
	return frame_len;
}

static struct audio_frame_t * audio_source_qoa_read(struct audio_source_t * s)
{
	struct audio_source_qoa_pdata_t * pdat = (struct audio_source_qoa_pdata_t *)s->priv;
	int src_index = pdat->sample_data_pos * pdat->info.channels;
	int dst_index = 0;
	int frames = 0;
	for(int i = 0; i < pdat->nsample / pdat->info.channels; i++)
	{
		if(pdat->sample_data_len - pdat->sample_data_pos == 0)
		{
			if(!decode_frame(pdat))
				break;
			src_index = 0;
		}
		for(int c = 0; c < pdat->info.channels; c++)
			pdat->samples[dst_index++] = pdat->sample_data[src_index++] * (pdat->factor / 32768.0);
		pdat->sample_data_pos++;
		pdat->sample_pos++;
		frames++;
	}
	pdat->output.rate = pdat->info.samplerate;
	pdat->output.channel = pdat->info.channels;
	pdat->output.frames = frames;
	pdat->output.samples = pdat->samples;
	return &pdat->output;
}

static int audio_source_qoa_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	struct audio_source_qoa_pdata_t * pdat = (struct audio_source_qoa_pdata_t *)s->priv;

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

static void audio_source_qoa_destroy(struct audio_source_t * s)
{
	struct audio_source_qoa_pdata_t * pdat = (struct audio_source_qoa_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->file)
			xfs_close(pdat->file);
		if(pdat->buffer)
			xos_mem_free(pdat->buffer);
		if(pdat->sample_data)
			xos_mem_free(pdat->sample_data);
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

struct audio_source_t * audio_source_alloc_from_xfs_qoa(struct xfs_context_t * ctx, const char * filename)
{
	struct xfs_file_t * file;
	struct qoa_desc_t info;
	uint8_t header[16];

	if(!(file = xfs_open_read(ctx, filename)))
		return NULL;

	if(xfs_read(file, &header, sizeof(header)) != sizeof(header))
	{
		xfs_close(file);
		return NULL;
	}

	unsigned int p = qoa_decode_header((const unsigned char *)header, sizeof(header), &info);
	if(!p)
	{
		xfs_close(file);
		return NULL;
	}

	struct audio_source_qoa_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_source_qoa_pdata_t));
	if(!pdat)
	{
		xfs_close(file);
		return NULL;
	}

	xos_memcpy(&pdat->info, &info, sizeof(struct qoa_desc_t));
	pdat->file = file;
	pdat->factor = 1.0;
	pdat->first_frame_offset = p;
	pdat->sample_data_len = 0;
	pdat->sample_data_pos = 0;
	pdat->sample_pos = 0;

	pdat->nbuffer = qoa_max_frame_size(&pdat->info);
	pdat->buffer = xos_mem_malloc(pdat->nbuffer);
	if(!pdat->buffer)
	{
		xfs_close(pdat->file);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->sample_data_size = pdat->info.channels * (256 * 20) * sizeof(short) * 2;
	pdat->sample_data = xos_mem_malloc(pdat->sample_data_size);
	if(!pdat->buffer)
	{
		xfs_close(pdat->file);
		xos_mem_free(pdat->buffer);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->nsample = pdat->info.samplerate * pdat->info.channels / 200;
	pdat->samples = xos_mem_malloc(pdat->nsample * sizeof(float));
	if(!pdat->samples)
	{
		xfs_close(pdat->file);
		xos_mem_free(pdat->buffer);
		xos_mem_free(pdat->sample_data);
		xos_mem_free(pdat);
		return NULL;
	}
	xfs_seek(pdat->file, pdat->first_frame_offset);

	struct audio_source_t * s = audio_source_alloc();
	if(!s)
	{
		xfs_close(pdat->file);
		xos_mem_free(pdat->buffer);
		xos_mem_free(pdat->sample_data);
		xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
		return NULL;
	}
	s->seek = audio_source_qoa_seek;
	s->tell = audio_source_qoa_tell;
	s->length = audio_source_qoa_length;
	s->read = audio_source_qoa_read;
	s->ioctl = audio_source_qoa_ioctl;
	s->destroy = audio_source_qoa_destroy;
	s->priv = pdat;

	return s;
}
