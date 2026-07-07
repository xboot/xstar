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

#include <kernel/time/timer.h>
#include <driver/pwm/pwm.h>
#include <driver/audio/playback.h>

struct playback_pwm_pdata_t {
	struct timer_t timer;
	struct fifo_t * fifo;
	struct pwm_t * pwm;
	int polarity;

	int rate;
	int channel;
	int running;
	float factor;
};

static int playback_pwm_resample(float * out, int osr, float * in, int isr, int isample, int ch, float factor)
{
	int osample = (int)(isample  * (float)osr / (float)isr);
	osample -= osample % ch;
	osample /= ch;
	float fixed = (1.0 / (1LL << 32));
	uint64_t frac = (1LL << 32);
	uint64_t step = ((uint64_t)(((float)isr / (float)osr) * frac + 0.5));
	uint64_t offset = 0;
	for(int i = 0; i < osample; i++)
	{
		float sum = 0.0;
		for(int c = 0; c < ch; c++)
			sum += (float)(in[c] + (in[c + ch] - in[c]) * ((float)(offset >> 32) + ((offset & (frac - 1)) * fixed))) * factor;
		*out++ = sum / ch;
		offset += step;
		in += (offset >> 32) * ch;
		offset &= (frac - 1);
	}
	return osample;
}

static int playback_pwm_timer_function(struct timer_t * timer, void * data)
{
	struct audio_playback_t * playback = (struct audio_playback_t *)(data);
	struct playback_pwm_pdata_t * pdat = (struct playback_pwm_pdata_t *)playback->priv;
	float v;

	if(pdat->running)
	{
		if((__fifo_available(pdat->fifo) >= 4) && (__fifo_get(pdat->fifo, (unsigned char *)&v, 4) == 4))
		{
			pwm_config(pdat->pwm, (int)((v + 1.0) * 8000), 16000, pdat->polarity);
			pwm_enable(pdat->pwm);
			timer_forward(&pdat->timer, us_to_ktime(125));
			return 1;
		}
	}
	pwm_disable(pdat->pwm);
	pdat->running = 0;
	return 0;
}

static int playback_pwm_start(struct audio_playback_t * playback, int rate, int channel)
{
	struct playback_pwm_pdata_t * pdat = (struct playback_pwm_pdata_t *)playback->priv;

	pdat->rate = rate;
	pdat->channel = channel;
	return 1;
}

static int playback_pwm_write(struct audio_playback_t * playback, float * samples, int nsample)
{
	struct playback_pwm_pdata_t * pdat = (struct playback_pwm_pdata_t *)playback->priv;
	float buffer[256];
	int len = 0;

	if(__fifo_available(pdat->fifo) >= 1024)
	{
		int l = XMIN(nsample, (int)ARRAY_SIZE(buffer) * pdat->channel);
		int isample = (l / pdat->channel) * pdat->channel;
		if(isample > 0)
		{
			int osample = playback_pwm_resample(buffer, 8000, samples, pdat->rate, isample, pdat->channel, pdat->factor);
			__fifo_put(pdat->fifo, (unsigned char *)buffer, osample * sizeof(float));
			len = isample;
		}
	}
	if(!pdat->running)
	{
		timer_start(&pdat->timer, ms_to_ktime(1));
		pdat->running = 1;
	}
	return len;
}

static int playback_pwm_stop(struct audio_playback_t * playback)
{
	struct playback_pwm_pdata_t * pdat = (struct playback_pwm_pdata_t *)playback->priv;

	pdat->running = 0;
	__fifo_reset(pdat->fifo);
	return 1;
}

static int playback_pwm_ioctl(struct audio_playback_t * playback, const char * cmd, void * arg)
{
	struct playback_pwm_pdata_t * pdat = (struct playback_pwm_pdata_t *)playback->priv;

	switch(shash(cmd))
	{
	case 0x0e6cf669: /* "audio-playback-set-volume" */
		if(arg)
		{
			int * p = arg;
			pdat->factor = volume_to_factor(p[0]);
			return 0;
		}
		break;

	case 0xddee28dd: /* "audio-playback-get-volume" */
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

static struct device_t * playback_pwm_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct playback_pwm_pdata_t * pdat;
	struct pwm_t * pwm;
	struct audio_playback_t * playback;
	struct device_t * dev;

	if(!(pwm = search_pwm(dt_read_string(n, "pwm-name", NULL))))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct playback_pwm_pdata_t));
	if(!pdat)
		return NULL;

	playback = xos_mem_malloc(sizeof(struct audio_playback_t));
	if(!playback)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	timer_init(&pdat->timer, playback_pwm_timer_function, playback);
	pdat->fifo = fifo_alloc(2048);
	pdat->pwm = pwm;
	pdat->polarity = dt_read_bool(n, "pwm-polarity", 1);
	pdat->running = 0;
	pdat->factor = 1.0;

	playback->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	playback->start = playback_pwm_start;
	playback->write = playback_pwm_write;
	playback->stop = playback_pwm_stop;
	playback->ioctl = playback_pwm_ioctl;
	playback->priv = pdat;

	if(!(dev = register_audio_playback(playback, drv)))
	{
		timer_cancel(&pdat->timer);
		fifo_free(pdat->fifo);
		free_device_name(playback->name);
		xos_mem_free(playback->priv);
		xos_mem_free(playback);
		return NULL;
	}
	return dev;
}

static void playback_pwm_remove(struct device_t * dev)
{
	struct audio_playback_t * playback = (struct audio_playback_t *)dev->priv;
	struct playback_pwm_pdata_t * pdat = (struct playback_pwm_pdata_t *)playback->priv;

	if(playback)
	{
		unregister_audio_playback(playback);
		timer_cancel(&pdat->timer);
		fifo_free(pdat->fifo);
		free_device_name(playback->name);
		xos_mem_free(playback->priv);
		xos_mem_free(playback);
	}
}

static void playback_pwm_suspend(struct device_t * dev)
{
}

static void playback_pwm_resume(struct device_t * dev)
{
}

static struct driver_t playback_pwm = {
	.name		= "playback-pwm",
	.probe		= playback_pwm_probe,
	.remove		= playback_pwm_remove,
	.suspend	= playback_pwm_suspend,
	.resume		= playback_pwm_resume,
};

static void playback_pwm_driver_init(void)
{
	register_driver(&playback_pwm);
}

static void playback_pwm_driver_exit(void)
{
	unregister_driver(&playback_pwm);
}

driver_initcall(playback_pwm_driver_init);
driver_exitcall(playback_pwm_driver_exit);
