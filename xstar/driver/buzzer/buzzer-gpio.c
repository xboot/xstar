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

#include <driver/gpio/gpio.h>
#include <driver/buzzer/buzzer.h>

/*
 * GPIO Buzzer - Buzzer Driver Using Generic Purpose Input Output
 *
 * Required properties:
 * - gpio: buzzer attached pin
 *
 * Optional properties:
 * - gpio-config: gpio pin configuration
 * - active-low: low level for active buzzer
 *
 * Example:
 *   "buzzer-gpio": {
 *       "gpio": 0,
 *       "gpio-config": -1,
 *       "active-low": true
 *   }
 */

struct buzzer_gpio_pdata_t {
	int gpio;
	int gpiocfg;
	int active_low;
	int frequency;
};

static void buzzer_gpio_set_frequency(struct buzzer_gpio_pdata_t * pdat, int frequency)
{
	if(frequency > 0)
		gpio_set_value(pdat->gpio, pdat->active_low ? 0 : 1);
	else
		gpio_set_value(pdat->gpio, pdat->active_low ? 1 : 0);
}

static void buzzer_gpio_set(struct buzzer_t * buzzer, int frequency)
{
	struct buzzer_gpio_pdata_t * pdat = (struct buzzer_gpio_pdata_t *)buzzer->priv;

	if(pdat->frequency != frequency)
	{
		buzzer_gpio_set_frequency(pdat, frequency);
		pdat->frequency = frequency;
	}
}

static int buzzer_gpio_get(struct buzzer_t * buzzer)
{
	struct buzzer_gpio_pdata_t * pdat = (struct buzzer_gpio_pdata_t *)buzzer->priv;
	return pdat->frequency;
}

static struct device_t * buzzer_gpio_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct buzzer_gpio_pdata_t * pdat;
	struct buzzer_t * buzzer;
	struct device_t * dev;

	if(!gpio_is_valid(dt_read_int(n, "gpio", -1)))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct buzzer_gpio_pdata_t));
	if(!pdat)
		return NULL;

	buzzer = xos_mem_malloc(sizeof(struct buzzer_t));
	if(!buzzer)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->gpio = dt_read_int(n, "gpio", -1);
	pdat->gpiocfg = dt_read_int(n, "gpio-config", -1);
	pdat->active_low = dt_read_bool(n, "active-low", 0);
	pdat->frequency = -1;

	buzzer->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	buzzer->set = buzzer_gpio_set;
	buzzer->get = buzzer_gpio_get;
	buzzer->priv = pdat;

	if(pdat->gpiocfg >= 0)
		gpio_set_cfg(pdat->gpio, pdat->gpiocfg);
	gpio_set_pull(pdat->gpio, pdat->active_low ? GPIO_PULL_UP :GPIO_PULL_DOWN);
	gpio_set_direction(pdat->gpio, GPIO_DIRECTION_OUTPUT);
	buzzer_gpio_set(buzzer, 0);

	if(!(dev = register_buzzer(buzzer, drv)))
	{
		free_device_name(buzzer->name);
		xos_mem_free(buzzer->priv);
		xos_mem_free(buzzer);
		return NULL;
	}
	return dev;
}

static void buzzer_gpio_remove(struct device_t * dev)
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

static void buzzer_gpio_suspend(struct device_t * dev)
{
	struct buzzer_t * buzzer = (struct buzzer_t *)dev->priv;
	struct buzzer_gpio_pdata_t * pdat = (struct buzzer_gpio_pdata_t *)buzzer->priv;
	buzzer_gpio_set_frequency(pdat, 0);
}

static void buzzer_gpio_resume(struct device_t * dev)
{
	struct buzzer_t * buzzer = (struct buzzer_t *)dev->priv;
	struct buzzer_gpio_pdata_t * pdat = (struct buzzer_gpio_pdata_t *)buzzer->priv;
	buzzer_gpio_set_frequency(pdat, pdat->frequency);
}

static struct driver_t buzzer_gpio = {
	.name		= "buzzer-gpio",
	.probe		= buzzer_gpio_probe,
	.remove		= buzzer_gpio_remove,
	.suspend	= buzzer_gpio_suspend,
	.resume		= buzzer_gpio_resume,
};

static void buzzer_gpio_driver_init(void)
{
	register_driver(&buzzer_gpio);
}

static void buzzer_gpio_driver_exit(void)
{
	unregister_driver(&buzzer_gpio);
}

driver_initcall(buzzer_gpio_driver_init);
driver_exitcall(buzzer_gpio_driver_exit);
