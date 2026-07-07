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

struct afsk_mod_t {
	int mark_hz;
	int space_hz;
	int sample_rate;
	int bit_rate;

	struct {
		unsigned int bitstream;
		unsigned int bitbuf;
		int numbits;

		int lastb;
		int ch_idx, bitmask;
		unsigned int ph, phinc, bitph;
		unsigned int datalen;
		unsigned char data[512];
	} hdlc;
};

struct audio_source_afsk_pdata_t {
	struct afsk_mod_t * mod;
	float factor;

	struct audio_frame_t output;
	float * samples;
	int nsample;
};

static struct afsk_mod_t * afsk_mod_alloc(int mark_hz, int space_hz, int sample_rate, int bit_rate)
{
	struct afsk_mod_t * mod;

	mod = xos_mem_malloc(sizeof(struct afsk_mod_t));
	if(mod)
	{
		xos_memset(mod, 0, sizeof(struct afsk_mod_t));

		mod->mark_hz = mark_hz;
		mod->space_hz = space_hz;
		mod->sample_rate = sample_rate;
		mod->bit_rate = bit_rate;
		mod->hdlc.bitmask = 1;
		return mod;
	}
	return NULL;
}

static void afsk_mod_free(struct afsk_mod_t * mod)
{
	if(mod)
		xos_mem_free(mod);
}

static void afsk_mod_hdlc_add_byte(struct afsk_mod_t * mod, unsigned char byte, int stuff)
{
	unsigned int mask1, mask2;
	unsigned int mask3;
	int i;

	if(mod->hdlc.numbits >= 8)
	{
		if(mod->hdlc.datalen >= sizeof(mod->hdlc.data))
			return;
		mod->hdlc.data[mod->hdlc.datalen++] = mod->hdlc.bitbuf;
		mod->hdlc.bitbuf >>= 8;
		mod->hdlc.numbits -= 8;
	}
	mod->hdlc.bitbuf |= byte << mod->hdlc.numbits;
	mod->hdlc.bitstream >>= 8;
	mod->hdlc.bitstream |= byte << 8;
	mask1 = 0x1f0;
	mask2 = 0x100;
	mask3 = 0xffffffff >> (31 - mod->hdlc.numbits);
	mod->hdlc.numbits += 8;
	if(!stuff)
		goto nostuff;
	for(i = 0; i < 8; i++, mask1 <<= 1, mask2 <<= 1, mask3 = (mask3 << 1) | 1)
	{
		if((mod->hdlc.bitstream & mask1) != mask1)
			continue;
		mod->hdlc.bitstream &= ~mask2;
		mod->hdlc.bitbuf = (mod->hdlc.bitbuf & mask3) | ((mod->hdlc.bitbuf & (~mask3)) << 1);
		mod->hdlc.numbits++;
		mask3 = (mask3 << 1) | 1;
	}
nostuff:
	if(mod->hdlc.numbits >= 8)
	{
		if(mod->hdlc.datalen >= sizeof(mod->hdlc.data))
			return;
		mod->hdlc.data[mod->hdlc.datalen++] = mod->hdlc.bitbuf;
		mod->hdlc.bitbuf >>= 8;
		mod->hdlc.numbits -= 8;
	}
}

static const uint16_t crc_ccitt_table[] = {
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

static inline uint16_t calc_crc_ccitt(unsigned char * buf, int len)
{
	uint16_t crc = 0xffff;

	for(; len > 0; len--)
		crc = (crc >> 8) ^ crc_ccitt_table[(crc ^ *buf++) & 0xff];
	crc ^= 0xffff;
	return (crc & 0xffff);
}

static void afsk_mod_hdlc_add_packet(struct afsk_mod_t * mod, unsigned char * pkt, int len)
{
	unsigned char packet[255];

	uint16_t crc = calc_crc_ccitt(pkt, len);
	xos_memcpy(packet, pkt, len);
	packet[len++] = (crc >> 0) & 0xff;
	packet[len++] = (crc >> 8) & 0xff;

	for(int i = 0; i < 64; i++)
		afsk_mod_hdlc_add_byte(mod, 0x7e, 0);
	for(int i = 0; i < len; i++)
		afsk_mod_hdlc_add_byte(mod, packet[i], 1);
	for(int i = 0; i < 8; i++)
		afsk_mod_hdlc_add_byte(mod, 0x7e, 0);
}

static int afsk_mod_hdlc_gen(struct afsk_mod_t * mod, float * buf, int len, float factor)
{
	int n = 0;

	if(!mod || (mod->hdlc.ch_idx < 0) || (mod->hdlc.ch_idx >= mod->hdlc.datalen))
		return 0;
	for(; len > 0; len--, buf++, n++)
	{
		mod->hdlc.bitph += 0x10000 * mod->bit_rate / mod->sample_rate;
		if(mod->hdlc.bitph >= 0x10000)
		{
			mod->hdlc.bitph &= 0xffff;
			mod->hdlc.bitmask <<= 1;
			if(mod->hdlc.bitmask >= 0x100)
			{
				mod->hdlc.bitmask = 1;
				mod->hdlc.ch_idx++;
				if(mod->hdlc.ch_idx >= mod->hdlc.datalen)
					return n;
			}
			if(!(mod->hdlc.data[mod->hdlc.ch_idx] & mod->hdlc.bitmask))
				mod->hdlc.lastb = !mod->hdlc.lastb;
			mod->hdlc.phinc = (mod->hdlc.lastb) ? 0x10000 * mod->space_hz / mod->sample_rate : 0x10000 * mod->mark_hz / mod->sample_rate;
		}
		*buf = fcos(mod->hdlc.ph) * factor;
		mod->hdlc.ph += mod->hdlc.phinc;
	}
	return n;
}

static int audio_source_afsk_seek(struct audio_source_t * s, int offset)
{
	struct audio_source_afsk_pdata_t * pdat = (struct audio_source_afsk_pdata_t *)s->priv;

	pdat->mod->hdlc.lastb = 0;
	pdat->mod->hdlc.ch_idx = 0;
	pdat->mod->hdlc.bitmask = 1;
	return 0;
}

static int audio_source_afsk_tell(struct audio_source_t * s)
{
	return 0;
}

static int audio_source_afsk_length(struct audio_source_t * s)
{
	return 0;
}

static struct audio_frame_t * audio_source_afsk_read(struct audio_source_t * s)
{
	struct audio_source_afsk_pdata_t * pdat = (struct audio_source_afsk_pdata_t *)s->priv;
	int frames = afsk_mod_hdlc_gen(pdat->mod, pdat->samples, pdat->nsample, pdat->factor);

	pdat->output.rate = pdat->mod->sample_rate;
	pdat->output.channel = 1;
	pdat->output.frames = frames;
	pdat->output.samples = pdat->samples;
	return &pdat->output;
}

static int audio_source_afsk_ioctl(struct audio_source_t * s, const char * cmd, void * arg)
{
	struct audio_source_afsk_pdata_t * pdat = (struct audio_source_afsk_pdata_t *)s->priv;

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

static void audio_source_afsk_destroy(struct audio_source_t * s)
{
	struct audio_source_afsk_pdata_t * pdat = (struct audio_source_afsk_pdata_t *)s->priv;

	if(pdat)
	{
		if(pdat->mod)
			afsk_mod_free(pdat->mod);
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
	}
}

struct audio_source_t * audio_source_alloc_afsk(int mark_hz, int space_hz, int sample_rate, int bit_rate, unsigned char * packet, int len)
{
	if(!packet || (len <= 0) || (len > 255 - 32 - 2))
		return NULL;

	struct audio_source_afsk_pdata_t * pdat = xos_mem_malloc(sizeof(struct audio_source_afsk_pdata_t));
	if(!pdat)
		return NULL;

	pdat->mod = afsk_mod_alloc(mark_hz, space_hz, XMAX(8000, sample_rate), bit_rate);
	if(!pdat->mod)
	{
		xos_mem_free(pdat);
		return NULL;
	}
	afsk_mod_hdlc_add_packet(pdat->mod, packet, len);

	pdat->factor = 1.0f;
	pdat->nsample = pdat->mod->sample_rate / 200;
	pdat->samples = xos_mem_malloc(pdat->nsample * sizeof(float));
	if(!pdat->samples)
	{
		if(pdat->mod)
			afsk_mod_free(pdat->mod);
		xos_mem_free(pdat);
		return NULL;
	}

	struct audio_source_t * s = audio_source_alloc();
	if(!s)
	{
		if(pdat->mod)
			afsk_mod_free(pdat->mod);
		if(pdat->samples)
			xos_mem_free(pdat->samples);
		xos_mem_free(pdat);
		return NULL;
	}
	s->seek = audio_source_afsk_seek;
	s->tell = audio_source_afsk_tell;
	s->length = audio_source_afsk_length;
	s->read = audio_source_afsk_read;
	s->ioctl = audio_source_afsk_ioctl;
	s->destroy = audio_source_afsk_destroy;
	s->priv = pdat;

	return s;
}
