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
#include <driver/vibrator/vibrator.h>

/*
 * PWM Vibrator - Vibrator Driver Using Pulse Width Modulation
 *
 * Required properties:
 * - pwm-name: vibrator attached pwm
 *
 * Optional properties:
 * - pwm-period-ns: pwm period in ns
 * - pwm-polarity: pwm polarity
 *
 * Example:
 *   "vibrator-pwm": {
 *       "pwm-name": "pwm.0",
 *       "pwm-period-ns": 2272727,
 *       "pwm-polarity": true
 *   }
 */

struct vibrator_pwm_pdata_t {
	struct pwm_t * pwm;
	int period;
	int polarity;
	int state;
};

static void vibrator_pwm_set_state(struct vibrator_pwm_pdata_t * pdat, int state)
{
	if(state > 0)
	{
		pwm_config(pdat->pwm, pdat->period / 2, pdat->period, pdat->polarity);
		pwm_enable(pdat->pwm);
	}
	else
	{
		pwm_disable(pdat->pwm);
	}
}

static void vibrator_pwm_set(struct vibrator_t * vib, int state)
{
	struct vibrator_pwm_pdata_t * pdat = (struct vibrator_pwm_pdata_t *)vib->priv;

	if(pdat->state != state)
	{
		vibrator_pwm_set_state(pdat, state);
		pdat->state = state;
	}
}

static int vibrator_pwm_get(struct vibrator_t * vib)
{
	struct vibrator_pwm_pdata_t * pdat = (struct vibrator_pwm_pdata_t *)vib->priv;
	return pdat->state;
}

static struct device_t * vibrator_pwm_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct vibrator_pwm_pdata_t * pdat;
	struct pwm_t * pwm;
	struct vibrator_t * vib;
	struct device_t * dev;

	if(!(pwm = search_pwm(dt_read_string(n, "pwm-name", NULL))))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct vibrator_pwm_pdata_t));
	if(!pdat)
		return NULL;

	vib = xos_mem_malloc(sizeof(struct vibrator_t));
	if(!vib)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->pwm = pwm;
	pdat->period = dt_read_int(n, "pwm-period-ns", 2272727);
	pdat->polarity = dt_read_bool(n, "pwm-polarity", 1);
	pdat->state = -1;

	vib->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	vib->set = vibrator_pwm_set;
	vib->get = vibrator_pwm_get;
	vib->priv = pdat;

	vibrator_pwm_set(vib, 0);

	if(!(dev = register_vibrator(vib, drv)))
	{
		free_device_name(vib->name);
		xos_mem_free(vib->priv);
		xos_mem_free(vib);
		return NULL;
	}
	return dev;
}

static void vibrator_pwm_remove(struct device_t * dev)
{
	struct vibrator_t * vib = (struct vibrator_t *)dev->priv;

	if(vib)
	{
		unregister_vibrator(vib);
		free_device_name(vib->name);
		xos_mem_free(vib->priv);
		xos_mem_free(vib);
	}
}

static void vibrator_pwm_suspend(struct device_t * dev)
{
	struct vibrator_t * vib = (struct vibrator_t *)dev->priv;
	struct vibrator_pwm_pdata_t * pdat = (struct vibrator_pwm_pdata_t *)vib->priv;
	vibrator_pwm_set_state(pdat, 0);
}

static void vibrator_pwm_resume(struct device_t * dev)
{
	struct vibrator_t * vib = (struct vibrator_t *)dev->priv;
	struct vibrator_pwm_pdata_t * pdat = (struct vibrator_pwm_pdata_t *)vib->priv;
	vibrator_pwm_set_state(pdat, pdat->state);
}

static struct driver_t vibrator_pwm = {
	.name		= "vibrator-pwm",
	.probe		= vibrator_pwm_probe,
	.remove		= vibrator_pwm_remove,
	.suspend	= vibrator_pwm_suspend,
	.resume		= vibrator_pwm_resume,
};

static void vibrator_pwm_driver_init(void)
{
	register_driver(&vibrator_pwm);
}

static void vibrator_pwm_driver_exit(void)
{
	unregister_driver(&vibrator_pwm);
}

driver_initcall(vibrator_pwm_driver_init);
driver_exitcall(vibrator_pwm_driver_exit);
