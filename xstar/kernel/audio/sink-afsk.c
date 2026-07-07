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

/*
 * Audio Frequency-Shift Keying (AFSK) Demodulation
 *
 * Modulation:
 *   - Bell 103 standard (1270Hz=1, 1070Hz=0) @ 300 bps
 *   - Bell 202 standard (1200Hz=1, 2200Hz=0) @ 1200 bps
 *
 * Encoding:
 *   NRZI (Non-Return to Zero Inverted) with HDLC framing
 *
 * Frame structure:
 *   [Flag]   - HDLC frame delimiter: 0x7E (01111110)
 *   [Packet] - Variable-length packet field
 *   [FCS]    - 16-bit CRC-CCITT (frame check sequence)
 *   [Flag]   - HDLC frame delimiter: 0x7E (01111110)
 *
 * Bit-level processing:
 *   - NRZI decoding: Bit transitions indicate '0', no transition indicates '1'
 *   - Bit stuffing: Remove inserted '0' after any five consecutive '1's
 *   - Bit order: LSB first within each byte
 *   - FCS validation: Polynomial x^16 + x^12 + x^5 + 1 (CCITT standard)
 */

struct afsk_demod_t {
	int mark_hz;
	int space_hz;
	int sample_rate;
	int bit_rate;
	int samples_per_bit;
	int subsamp;
	uint32_t sphaseinc;

	struct {
		struct {
			float * i;
			float * q;
		} mark;
		struct {
			float * i;
			float * q;
		} space;
	} corr;

	struct {
		uint32_t dcd_shreg;
		uint32_t sphase;
		uint32_t lasts;
		uint32_t subsamp;
	} afsk;

	struct {
		uint8_t rxbuf[512];
		uint8_t * rxptr;
		uint32_t rxstate;
		uint32_t rxbitstream;
		uint32_t rxbitbuf;
	} hdlc;
};

struct audio_sink_afsk_pdata_t {
	struct afsk_demod_t * demod;
	struct audio_filter_t * filter;

	float * buffer;
	int buflen;
	int bufidx;
};

static struct afsk_demod_t * afsk_demod_alloc(int mark_hz, int space_hz, int sample_rate, int bit_rate)
{
	struct afsk_demod_t * demod;

	demod = xos_mem_malloc(sizeof(struct afsk_demod_t));
	if(demod)
	{
		xos_memset(demod, 0, sizeof(struct afsk_demod_t));

		demod->mark_hz = mark_hz;
		demod->space_hz = space_hz;
		demod->sample_rate = sample_rate;
		demod->bit_rate = bit_rate;
		demod->samples_per_bit = (int)(demod->sample_rate / demod->bit_rate);
		demod->subsamp = 2;
		demod->sphaseinc = 0x10000u * demod->bit_rate * demod->subsamp / demod->sample_rate;

		demod->corr.mark.i = xos_mem_malloc(sizeof(float) * demod->samples_per_bit);
		demod->corr.mark.q = xos_mem_malloc(sizeof(float) * demod->samples_per_bit);
		demod->corr.space.i = xos_mem_malloc(sizeof(float) * demod->samples_per_bit);
		demod->corr.space.q = xos_mem_malloc(sizeof(float) * demod->samples_per_bit);

		float u = 0.0f;
		for(int i = 0; i < demod->samples_per_bit; i++)
		{
			demod->corr.mark.i[i] = cosf(u);
			demod->corr.mark.q[i] = sinf(u);
			u += 2.0 * M_PI * demod->mark_hz / demod->sample_rate;
		}
		float v = 0.0f;
		for(int i = 0; i < demod->samples_per_bit; i++)
		{
			demod->corr.space.i[i] = cosf(v);
			demod->corr.space.q[i] = sinf(v);
			v += 2.0 * M_PI * demod->space_hz / demod->sample_rate;
		}
		return demod;
	}
	return NULL;
}

static void afsk_demod_free(struct afsk_demod_t * demod)
{
	if(demod)
	{
		if(demod->corr.mark.i)
			xos_mem_free(demod->corr.mark.i);
		if(demod->corr.mark.q)
			xos_mem_free(demod->corr.mark.q);
		if(demod->corr.space.i)
			xos_mem_free(demod->corr.space.i);
		if(demod->corr.space.q)
			xos_mem_free(demod->corr.space.q);
		xos_mem_free(demod);
	}
}

static const unsigned short crc_ccitt_table[] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78,
};

static inline int check_crc_ccitt(unsigned char * buf, int cnt)
{
	unsigned int crc = 0xffff;

	for(; cnt > 0; cnt--)
		crc = (crc >> 8) ^ crc_ccitt_table[(crc ^ *buf++) & 0xff];
	return (crc & 0xffff) == 0xf0b8;
}

static void afsk_demod_hdlc_rxbit(struct afsk_demod_t * demod, int bit)
{
	demod->hdlc.rxbitstream <<= 1;
	demod->hdlc.rxbitstream |= !!bit;
	if((demod->hdlc.rxbitstream & 0xff) == 0x7e)
	{
		if(demod->hdlc.rxstate && ((demod->hdlc.rxptr - demod->hdlc.rxbuf) > 2))
		{
			unsigned char * bp = demod->hdlc.rxbuf;
			unsigned int len = demod->hdlc.rxptr - demod->hdlc.rxbuf;
			if(len > 2)
			{
				if(check_crc_ccitt(bp, len))
					psub_publish("audiosink.afsk", (void *)&(struct{unsigned char * buf;unsigned int len;}){ bp, len - 2});
			}
		}
		demod->hdlc.rxstate = 1;
		demod->hdlc.rxptr = demod->hdlc.rxbuf;
		demod->hdlc.rxbitbuf = 0x80;
		return;
	}
	if((demod->hdlc.rxbitstream & 0x7f) == 0x7f)
	{
		demod->hdlc.rxstate = 0;
		return;
	}
	if(!demod->hdlc.rxstate)
		return;
	if((demod->hdlc.rxbitstream & 0x3f) == 0x3e)
		return;
	if(demod->hdlc.rxbitstream & 1)
		demod->hdlc.rxbitbuf |= 0x100;
	if(demod->hdlc.rxbitbuf & 1)
	{
		if(demod->hdlc.rxptr >= demod->hdlc.rxbuf + sizeof(demod->hdlc.rxbuf))
		{
			demod->hdlc.rxstate = 0;
			return;
		}
		*demod->hdlc.rxptr++ = demod->hdlc.rxbitbuf >> 1;
		demod->hdlc.rxbitbuf = 0x80;
		return;
	}
	demod->hdlc.rxbitbuf >>= 1;
}

static inline float fsqr(float f)
{
	return f * f;
}

static inline float mac(const float * a, const float * b, unsigned int size)
{
	float sum = 0.0f;

	for(unsigned int i = 0; i < size; i++)
		sum += (*a++) * (*b++);
	return sum;
}

static void afsk_demod_process(struct afsk_demod_t * demod, float * buffer, int length)
{
	if(demod->afsk.subsamp)
	{
		if(length <= (int)demod->afsk.subsamp)
		{
			demod->afsk.subsamp -= length;
			return;
		}
		buffer += demod->afsk.subsamp;
		length -= demod->afsk.subsamp;
		demod->afsk.subsamp = 0;
	}
	for(; length > 0; length -= demod->subsamp, buffer += demod->subsamp)
	{
		float f = fsqr(mac(buffer, demod->corr.mark.i, demod->samples_per_bit)) +
			fsqr(mac(buffer, demod->corr.mark.q, demod->samples_per_bit)) -
			fsqr(mac(buffer, demod->corr.space.i, demod->samples_per_bit)) -
			fsqr(mac(buffer, demod->corr.space.q, demod->samples_per_bit));
		demod->afsk.dcd_shreg <<= 1;
		demod->afsk.dcd_shreg |= (f > 0);

		if((demod->afsk.dcd_shreg ^ (demod->afsk.dcd_shreg >> 1)) & 1)
		{
			if(demod->afsk.sphase < (0x8000u - (demod->sphaseinc / 2)))
				demod->afsk.sphase += demod->sphaseinc / 8;
			else
				demod->afsk.sphase -= demod->sphaseinc / 8;
		}
		demod->afsk.sphase += demod->sphaseinc;
		if(demod->afsk.sphase >= 0x10000u)
		{
			demod->afsk.sphase &= 0xffffu;
			demod->afsk.lasts <<= 1;
			demod->afsk.lasts |= demod->afsk.dcd_shreg & 1;
			afsk_demod_hdlc_rxbit(demod, (demod->afsk.lasts ^ (demod->afsk.lasts >> 1) ^ 1) & 1);
		}
	}
	demod->afsk.subsamp = -length;
}

static void audio_sink_afsk_write(struct audio_sink_t * s, struct audio_frame_t * af)
{
	struct audio_sink_afsk_pdata_t * pdat = (struct audio_sink_afsk_pdata_t *)s->priv;
	struct audio_frame_t * output = audio_filter_process(pdat->filter, af);
	float * po = output->samples;

	for(int n = 0; n < output->frames; n++)
	{
		pdat->buffer[pdat->bufidx++] = po[n];
		if(pdat->bufidx >= pdat->buflen)
		{
			afsk_demod_process(pdat->demod, pdat->buffer, pdat->buflen - pdat->demod->samples_per_bit);
			xos_memmove(&pdat->buffer[0], &pdat->buffer[pdat->buflen - pdat->demod->samples_per_bit], pdat->demod->samples_per_bit * sizeof(float));
			pdat->bufidx = pdat->demod->samples_per_bit;
		}
	}
}

static int audio_sink_afsk_ioctl(struct audio_sink_t * s, const char * cmd, void * arg)
{
	return -1;
}

static void audio_sink_afsk_destroy(struct audio_sink_t * s)
{
	struct audio_sink_afsk_pdata_t * pdat = (struct audio_sink_afsk_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->demod)
			afsk_demod_free(pdat->demod);
		if(pdat->filter)
			audio_filter_free(pdat->filter);
		if(pdat->buffer)
			xos_mem_free(pdat->buffer);
		xos_mem_free(pdat);
	}
}

struct audio_sink_t * audio_sink_alloc_afsk(int mark_hz, int space_hz, int sample_rate, int bit_rate)
{
	struct audio_sink_afsk_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_sink_afsk_pdata_t));
	if(!pdat)
		return NULL;

	pdat->demod = afsk_demod_alloc(mark_hz, space_hz, XMAX(8000, sample_rate), bit_rate);
	if(!pdat->demod)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	char json[256];
	int length = xos_sprintf(json, "{\"mono\":{},\"resample\":{\"rate\":%d}}", pdat->demod->sample_rate);
	pdat->filter = audio_filter_alloc(json, length);
	if(!pdat->filter)
	{
		if(pdat->demod)
			afsk_demod_free(pdat->demod);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->buflen = 8192;
	pdat->buffer = xos_mem_malloc(sizeof(float) * pdat->buflen);
	if(!pdat->buffer)
	{
		if(pdat->demod)
			afsk_demod_free(pdat->demod);
		if(pdat->filter)
			audio_filter_free(pdat->filter);
		xos_mem_free(pdat);
		return NULL;
	}
	pdat->bufidx = 0;

	struct audio_sink_t * s = audio_sink_alloc();
	if(!s)
	{
		if(pdat->demod)
			afsk_demod_free(pdat->demod);
		if(pdat->filter)
			audio_filter_free(pdat->filter);
		if(pdat->buffer)
			xos_mem_free(pdat->buffer);
		xos_mem_free(pdat);
		return NULL;
	}
	s->write = audio_sink_afsk_write;
	s->ioctl = audio_sink_afsk_ioctl;
	s->destroy = audio_sink_afsk_destroy;
	s->priv = pdat;

	return s;
}
