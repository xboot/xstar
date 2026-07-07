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

#include <kernel/audio/effect.h>

/*
 * IIR-Biquad - Infinite impulse response biquadratic filter.
 *
 * Online designer:
 *    https://arachnoid.com/BiQuadDesigner
 *
 * Example:
 *   "iir": {
 *       "type" : "lowpass",
 *       "frequency" : 1000.0,
 *       "quality" : 0.707
 *   }
 */

enum iir_biquad_type_t {
	IIR_BIQUAD_TYPE_LOWPASS,
	IIR_BIQUAD_TYPE_HIGHPASS,
	IIR_BIQUAD_TYPE_BANDPASS,
	IIR_BIQUAD_TYPE_LOWSHELF,
	IIR_BIQUAD_TYPE_HIGHSHELF,
	IIR_BIQUAD_TYPE_PEAKING,
	IIR_BIQUAD_TYPE_NOTCH,
	IIR_BIQUAD_TYPE_ALLPASS,
};

struct iir_biquad_coeff_t {
	float b0, b1, b2;
	float a1, a2;
};

struct iir_biquad_state_t {
	float xn1, xn2;
	float yn1, yn2;
};

struct iir_biquad_pdata_t {
	struct iir_biquad_coeff_t coeff;
	struct iir_biquad_state_t * state;
	int nstate;

	enum iir_biquad_type_t type;
	float frequency;
	float quality;
	float gain;
	int rate;
	int __setup;
};

static void * iir_biquad_create(void)
{
	struct iir_biquad_pdata_t * pdat;

	pdat = xos_mem_malloc(sizeof(struct iir_biquad_pdata_t));
	if(!pdat)
		return NULL;

	pdat->coeff.b0 = 0.00391608f;
	pdat->coeff.b1 = 0.00783215f;
	pdat->coeff.b2 = 0.00391608f;
	pdat->coeff.a1 = -1.81531792f;
	pdat->coeff.a2 = 0.83098222f;
	pdat->state = NULL;
	pdat->nstate = 0;

	pdat->type = IIR_BIQUAD_TYPE_LOWPASS;
	pdat->frequency = 1000.0f;
	pdat->quality = 0.707f;
	pdat->gain = 6.0f;
	pdat->rate = 48000;
	pdat->__setup = 1;

	return pdat;
}

static void iir_biquad_reinit(struct iir_biquad_pdata_t * pdat)
{
	if(pdat)
	{
		double omega = 2.0 * M_PI * pdat->frequency / (float)pdat->rate;
		double cs = cos(omega);
		double sn = sin(omega);
		double a1pha = sn / (2.0 * pdat->quality);
		double A = exp(log(10.0) * pdat->gain / 40.0);
		double beta = sqrt(A + A);

		float b0 = 0.00391608f, b1 = 0.00783215f, b2 = 0.00391608f;
		float a0 = 1.0f, a1 = -1.81531792f, a2 = 0.83098222f;

		switch(pdat->type)
		{
		case IIR_BIQUAD_TYPE_LOWPASS:
			b0 = (1.0 - cs) / 2.0;
			b1 = 1.0 - cs;
			b2 = (1.0 - cs) / 2.0;
			a0 = 1.0 + a1pha;
			a1 = -2.0 * cs;
			a2 = 1.0 - a1pha;
			break;

		case IIR_BIQUAD_TYPE_HIGHPASS:
			b0 = (1.0 + cs) / 2.0;
			b1 = -(1.0 + cs);
			b2 = (1.0 + cs) / 2.0;
			a0 = 1.0 + a1pha;
			a1 = -2.0 * cs;
			a2 = 1.0 - a1pha;
			break;

		case IIR_BIQUAD_TYPE_BANDPASS:
			b0 = a1pha;
			b1 = 0.0;
			b2 = -a1pha;
			a0 = 1.0 + a1pha;
			a1 = -2.0 * cs;
			a2 = 1.0 - a1pha;
			break;

		case IIR_BIQUAD_TYPE_LOWSHELF:
			b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
			b1 = 2 * A * ((A - 1) - (A + 1) * cs);
			b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
			a0 = (A + 1) + (A - 1) * cs + beta * sn;
			a1 = -2 * ((A - 1) + (A + 1) * cs);
			a2 = (A + 1) + (A - 1) * cs - beta * sn;
			break;

		case IIR_BIQUAD_TYPE_HIGHSHELF:
			b0 = A * ((A + 1.0) + (A - 1.0) * cs + beta * sn);
			b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cs);
			b2 = A * ((A + 1.0) + (A - 1.0) * cs - beta * sn);
			a0 = (A + 1.0) - (A - 1.0) * cs + beta * sn;
			a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cs);
			a2 = (A + 1.0) - (A - 1.0) * cs - beta * sn;
			break;

		case IIR_BIQUAD_TYPE_PEAKING:
			b0 = 1.0 + a1pha * A;
			b1 = -2.0 * cs;
			b2 = 1.0 - a1pha * A;
			a0 = 1.0 + a1pha / A;
			a1 = -2.0 * cs;
			a2 = 1.0 - a1pha / A;
			break;

		case IIR_BIQUAD_TYPE_NOTCH:
			b0 = 1.0;
			b1 = -2.0 * cs;
			b2 = 1.0;
			a0 = 1.0 + a1pha;
			a1 = -2.0 * cs;
			a2 = 1.0 - a1pha;
			break;

		case IIR_BIQUAD_TYPE_ALLPASS:
			b0 = 1.0 - a1pha;
			b1 = -2.0 * cs;
			b2 = 1.0 + a1pha;
			a0 = 1.0 + a1pha;
			a1 = -2.0 * cs;
			a2 = 1.0 - a1pha;
			break;

		default:
			break;
		}

		pdat->coeff.b0 = b0 / a0;
		pdat->coeff.b1 = b1 / a0;
		pdat->coeff.b2 = b2 / a0;
		pdat->coeff.a1 = a1 / a0;
		pdat->coeff.a2 = a2 / a0;
		if(pdat->state && (pdat->nstate > 0))
			xos_memset(pdat->state, 0, sizeof(struct iir_biquad_state_t) * pdat->nstate);
	}
}

static void iir_biquad_setup(void * ctx, struct json_value_t * v)
{
	struct iir_biquad_pdata_t * pdat = (struct iir_biquad_pdata_t *)ctx;

	if(v && (v->type == JSON_OBJECT))
	{
		for(int i = 0; i < v->u.object.length; i++)
		{
			struct json_value_t * o = v->u.object.values[i].value;
			switch(shash(v->u.object.values[i].name))
			{
			case 0x7c9ebd07: /* "type" */
				if(o && (o->type == JSON_STRING))
				{
					pdat->__setup = 1;
					switch(shash((char *)o->u.string.ptr))
					{
					case 0x88e656ee: /* "lowpass" */
						pdat->type = IIR_BIQUAD_TYPE_LOWPASS;
						break;
					case 0x00f1141c: /* "highpass" */
						pdat->type = IIR_BIQUAD_TYPE_HIGHPASS;
						break;
					case 0x1f9feb31: /* "bandpass" */
						pdat->type = IIR_BIQUAD_TYPE_BANDPASS;
						break;
					case 0xa5eb18c9: /* "lowshelf" */
						pdat->type = IIR_BIQUAD_TYPE_LOWSHELF;
						break;
					case 0x1f4d7bb7: /* "highshelf" */
						pdat->type = IIR_BIQUAD_TYPE_HIGHSHELF;
						break;
					case 0xa3eafc04: /* "peaking" */
						pdat->type = IIR_BIQUAD_TYPE_PEAKING;
						break;
					case 0x10046041: /* "notch" */
						pdat->type = IIR_BIQUAD_TYPE_NOTCH;
						break;
					case 0x325f3335: /* "allpass" */
						pdat->type = IIR_BIQUAD_TYPE_ALLPASS;
						break;
					default:
						pdat->__setup = 0;
						break;
					}
				}
				break;

			case 0xffd1bad7: /* "frequency" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->frequency = (float)o->u.dbl;
					pdat->__setup = 1;
				}
				break;

			case 0x1638500e: /* "quality" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->quality = XMAX((float)o->u.dbl, (float)1e-9);
					pdat->__setup = 1;
				}
				break;

			case 0x7c973524: /* "gain" */
				if(o && (o->type == JSON_DOUBLE))
				{
					pdat->gain = (float)o->u.dbl;
					pdat->__setup = 1;
				}
				break;

			default:
				break;
			}
		}
	}
}

static void iir_biquad_prepare(void * ctx, struct audio_frame_t * input)
{
	struct iir_biquad_pdata_t * pdat = (struct iir_biquad_pdata_t *)ctx;

	if(input->channel > pdat->nstate)
	{
		pdat->nstate = input->channel;
		pdat->state = xos_mem_realloc(pdat->state, sizeof(struct iir_biquad_state_t) * pdat->nstate);
		xos_memset(pdat->state, 0, sizeof(struct iir_biquad_state_t) * pdat->nstate);
	}
	if((input->rate != pdat->rate) || pdat->__setup)
	{
		pdat->rate = input->rate;
		pdat->__setup = 0;
		iir_biquad_reinit(pdat);
	}
}

static struct audio_frame_t * iir_biquad_process(void * ctx, struct audio_frame_t * input)
{
	struct iir_biquad_pdata_t * pdat = (struct iir_biquad_pdata_t *)ctx;
	struct iir_biquad_coeff_t * coeff = &pdat->coeff;
	float * pi = input->samples;

	for(int n = 0; n < input->frames; n++)
	{
		for(int c = 0; c < input->channel; c++)
		{
			struct iir_biquad_state_t * state = &pdat->state[c];
			float x = pi[c];
			float y = coeff->b0 * x + coeff->b1 * state->xn1 + coeff->b2 * state->xn2 - coeff->a1 * state->yn1 - coeff->a2 * state->yn2;
			state->xn2 = state->xn1;
			state->xn1 = x;
			state->yn2 = state->yn1;
			state->yn1 = y;
			pi[c] = y;
		}
		pi += input->channel;
	}
	return input;
}

static void iir_biquad_destroy(void * ctx)
{
	struct iir_biquad_pdata_t * pdat = (struct iir_biquad_pdata_t *)ctx;

	if(pdat)
	{
		if(pdat->state)
			xos_mem_free(pdat->state);
		xos_mem_free(pdat);
	}
}

static struct audio_effect_t iir = {
	.name		= "iir",
	.create		= iir_biquad_create,
	.setup		= iir_biquad_setup,
	.prepare	= iir_biquad_prepare,
	.process	= iir_biquad_process,
	.destroy	= iir_biquad_destroy,
};

static void effect_iir_biquad_init(void)
{
	register_audio_effect(&iir);
}

static void effect_iir_biquad_exit(void)
{
	unregister_audio_effect(&iir);
}

core_initcall(effect_iir_biquad_init);
core_exitcall(effect_iir_biquad_exit);
