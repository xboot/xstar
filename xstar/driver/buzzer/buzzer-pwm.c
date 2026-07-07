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

#include <driver/pwm/pwm.h>
#include <driver/buzzer/buzzer.h>

/*
 * PWM Buzzer - Buzzer Driver Using Pulse Width Modulation
 *
 * Required properties:
 * - pwm-name: buzzer attached pwm
 *
 * Optional properties:
 * - pwm-polarity: pwm polarity
 *
 * Example:
 *   "buzzer-pwm": {
 *       "pwm-name": "pwm.0",
 *       "pwm-polarity": false
 *   }
 */

struct buzzer_pwm_pdata_t {
	struct pwm_t * pwm;
	int polarity;
	int frequency;
};

static void buzzer_pwm_set_frequency(struct buzzer_pwm_pdata_t * pdat, int frequency)
{
	if(frequency > 0)
	{
		int period = 1000000000ULL / frequency;
		pwm_config(pdat->pwm, period / 2, period, pdat->polarity);
		pwm_enable(pdat->pwm);
	}
	else
	{
		pwm_disable(pdat->pwm);
	}
}

static void buzzer_pwm_set(struct buzzer_t * buzzer, int frequency)
{
	struct buzzer_pwm_pdata_t * pdat = (struct buzzer_pwm_pdata_t *)buzzer->priv;

	if(pdat->frequency != frequency)
	{
		buzzer_pwm_set_frequency(pdat, frequency);
		pdat->frequency = frequency;
	}
}

static int buzzer_pwm_get(struct buzzer_t * buzzer)
{
	struct buzzer_pwm_pdata_t * pdat = (struct buzzer_pwm_pdata_t *)buzzer->priv;
	return pdat->frequency;
}

static struct device_t * buzzer_pwm_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct buzzer_pwm_pdata_t * pdat;
	struct pwm_t * pwm;
	struct buzzer_t * buzzer;
	struct device_t * dev;

	if(!(pwm = search_pwm(dt_read_string(n, "pwm-name", NULL))))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct buzzer_pwm_pdata_t));
	if(!pdat)
		return NULL;

	buzzer = xos_mem_malloc(sizeof(struct buzzer_t));
	if(!buzzer)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->pwm = pwm;
	pdat->polarity = dt_read_bool(n, "pwm-polarity", 0);
	pdat->frequency = -1;

	buzzer->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	buzzer->set = buzzer_pwm_set;
	buzzer->get = buzzer_pwm_get;
	buzzer->priv = pdat;

	buzzer_pwm_set(buzzer, 0);

	if(!(dev = register_buzzer(buzzer, drv)))
	{
		free_device_name(buzzer->name);
		xos_mem_free(buzzer->priv);
		xos_mem_free(buzzer);
		return NULL;
	}
	return dev;
}

static void buzzer_pwm_remove(struct device_t * dev)
{
	struct buzzer_t * buzzer = (struct buzzer_t *)dev->priv;

	if(buzzer)
	{
		unregister_buzzer(buzzer);
		free_device_name(buzzer->name);
		xos_mem_free(buzzer->priv);
		xos_mem_free(buzzer);
	}
}

static void buzzer_pwm_suspend(struct device_t * dev)
{
	struct buzzer_t * buzzer = (struct buzzer_t *)dev->priv;
	struct buzzer_pwm_pdata_t * pdat = (struct buzzer_pwm_pdata_t *)buzzer->priv;
	buzzer_pwm_set_frequency(pdat, 0);
}

static void buzzer_pwm_resume(struct device_t * dev)
{
	struct buzzer_t * buzzer = (struct buzzer_t *)dev->priv;
	struct buzzer_pwm_pdata_t * pdat = (struct buzzer_pwm_pdata_t *)buzzer->priv;
	buzzer_pwm_set_frequency(pdat, pdat->frequency);
}

static struct driver_t buzzer_pwm = {
	.name		= "buzzer-pwm",
	.probe		= buzzer_pwm_probe,
	.remove		= buzzer_pwm_remove,
	.suspend	= buzzer_pwm_suspend,
	.resume		= buzzer_pwm_resume,
};

static void buzzer_pwm_driver_init(void)
{
	register_driver(&buzzer_pwm);
}

static void buzzer_pwm_driver_exit(void)
{
	unregister_driver(&buzzer_pwm);
}

driver_initcall(buzzer_pwm_driver_init);
driver_exitcall(buzzer_pwm_driver_exit);
