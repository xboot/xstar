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

#include <driver/ledstrip/ledstrip.h>
#include <driver/led/led.h>

/*
 * LED STRIP - LED driver using ledstrip
 *
 * Required properties:
 * - ledstrip-name: the attached ledstrip
 *
 * Optional properties:
 * - color: led default color
 * - default-brightness: led default brightness
 *
 * Example:
 *   "led-lstrip:0": {
 *       "ledstrip-name": "ledstrip-sk9822.0",
 *       "color": "red",
 *       "default-brightness": 0
 *   }
 */

struct led_lstrip_pdata_t {
	struct ledstrip_t * strip;
	int brightness;
	float h, s, v;
};

static void led_lstrip_set_brightness(struct led_lstrip_pdata_t * pdat, int brightness)
{
	struct color_t c;

	color_set_hsva(&c, pdat->h, pdat->s, brightness * 0.001f, 1.0f);
	for(int i = 0; i < ledstrip_get_count(pdat->strip); i++)
		ledstrip_set_color(pdat->strip, i, &c);
	ledstrip_refresh(pdat->strip);
}

static void led_lstrip_set(struct led_t * led, int brightness)
{
	struct led_lstrip_pdata_t * pdat = (struct led_lstrip_pdata_t *)led->priv;

	if(pdat->brightness != brightness)
	{
		led_lstrip_set_brightness(pdat, brightness);
		pdat->brightness = brightness;
	}
}

static int led_lstrip_get(struct led_t * led)
{
	struct led_lstrip_pdata_t * pdat = (struct led_lstrip_pdata_t *)led->priv;
	return pdat->brightness;
}

static struct device_t * led_lstrip_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct led_lstrip_pdata_t * pdat;
	struct ledstrip_t * strip;
	struct led_t * led;
	struct device_t * dev;
	struct color_t col;

	if(!(strip = search_ledstrip(dt_read_string(n, "ledstrip-name", NULL))))
		return NULL;
	color_init_string(&col, dt_read_string(n, "color", "white"));

	pdat = xos_mem_malloc(sizeof(struct led_lstrip_pdata_t));
	if(!pdat)
		return NULL;

	led = xos_mem_malloc(sizeof(struct led_t));
	if(!led)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->strip = strip;
	pdat->brightness = -1;
	color_get_hsva(&col, &pdat->h, &pdat->s, &pdat->v, NULL);

	led->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	led->set = led_lstrip_set;
	led->get = led_lstrip_get;
	led->priv = pdat;

	led_lstrip_set(led, dt_read_int(n, "default-brightness", 0));

	if(!(dev = register_led(led, drv)))
	{
		free_device_name(led->name);
		xos_mem_free(led->priv);
		xos_mem_free(led);
		return NULL;
	}
	return dev;
}

static void led_lstrip_remove(struct device_t * dev)
{
	struct led_t * led = (struct led_t *)dev->priv;

	if(led)
	{
		unregister_led(led);
		free_device_name(led->name);
		xos_mem_free(led->priv);
		xos_mem_free(led);
	}
}

static void led_lstrip_suspend(struct device_t * dev)
{
	struct led_t * led = (struct led_t *)dev->priv;
	struct led_lstrip_pdata_t * pdat = (struct led_lstrip_pdata_t *)led->priv;
	led_lstrip_set_brightness(pdat, 0);
}

static void led_lstrip_resume(struct device_t * dev)
{
	struct led_t * led = (struct led_t *)dev->priv;
	struct led_lstrip_pdata_t * pdat = (struct led_lstrip_pdata_t *)led->priv;
	led_lstrip_set_brightness(pdat, pdat->brightness);
}

static struct driver_t led_lstrip = {
	.name		= "led-lstrip",
	.probe		= led_lstrip_probe,
	.remove		= led_lstrip_remove,
	.suspend	= led_lstrip_suspend,
	.resume		= led_lstrip_resume,
};

static void led_lstrip_driver_init(void)
{
	register_driver(&led_lstrip);
}

static void led_lstrip_driver_exit(void)
{
	unregister_driver(&led_lstrip);
}

driver_initcall(led_lstrip_driver_init);
driver_exitcall(led_lstrip_driver_exit);
