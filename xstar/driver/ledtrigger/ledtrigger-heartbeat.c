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
#include <driver/led/led.h>
#include <driver/ledtrigger/ledtrigger.h>

struct ledtrigger_heartbeat_pdata_t {
	struct timer_t timer;
	struct led_t * led;
	unsigned int phase;
	unsigned int period;
};

static int ledtrigger_heartbeat_timer_function(struct timer_t * timer, void * data)
{
	struct ledtrigger_t * trigger = (struct ledtrigger_t *)(data);
	struct ledtrigger_heartbeat_pdata_t * pdat = (struct ledtrigger_heartbeat_pdata_t *)trigger->priv;
	int loadavg = 0;
	int brightness;
	int delay;

	/*
	 * Acts like an actual heart beat -- thump-thump-pause ...
	 * f(0)=1260, f(1)=860, f(5)=510, f(inf)->300.
	 */
	switch(pdat->phase)
	{
	case 0:
		pdat->period = 300 + (6720 << 11) / (5 * loadavg + (7 << 11));
		delay = 70;
		pdat->phase++;
		brightness = 1000;
		break;

	case 1:
		delay = (pdat->period >> 2) - 70;
		pdat->phase++;
		brightness = 0;
		break;

	case 2:
		delay = 70;
		pdat->phase++;
		brightness = 1000;
		break;

	default:
		delay = pdat->period - (pdat->period >> 2) - 70;
		pdat->phase = 0;
		brightness = 0;
		break;
	}
	led_set_brightness(pdat->led, brightness);
	timer_forward(timer, ms_to_ktime(delay));
	return 1;
}

static struct device_t * ledtrigger_heartbeat_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct ledtrigger_heartbeat_pdata_t * pdat;
	struct ledtrigger_t * trigger;
	struct device_t * dev;
	struct led_t * led;

	led = search_led(dt_read_string(n, "led-name", NULL));
	if(!led)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct ledtrigger_heartbeat_pdata_t));
	if(!pdat)
		return NULL;

	trigger = xos_mem_malloc(sizeof(struct ledtrigger_t));
	if(!trigger)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	timer_init(&pdat->timer, ledtrigger_heartbeat_timer_function, trigger);
	pdat->led = led;
	pdat->phase = 0;
	pdat->period = 1260;

	trigger->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	trigger->priv = pdat;
	timer_start(&pdat->timer, ms_to_ktime(100));

	if(!(dev = register_ledtrigger(trigger, drv)))
	{
		timer_cancel(&pdat->timer);
		free_device_name(trigger->name);
		xos_mem_free(trigger->priv);
		xos_mem_free(trigger);
		return NULL;
	}
	return dev;
}

static void ledtrigger_heartbeat_remove(struct device_t * dev)
{
	struct ledtrigger_t * trigger = (struct ledtrigger_t *)dev->priv;
	struct ledtrigger_heartbeat_pdata_t * pdat = (struct ledtrigger_heartbeat_pdata_t *)trigger->priv;

	if(trigger)
	{
		unregister_ledtrigger(trigger);
		timer_cancel(&pdat->timer);
		free_device_name(trigger->name);
		xos_mem_free(trigger->priv);
		xos_mem_free(trigger);
	}
}

static void ledtrigger_heartbeat_suspend(struct device_t * dev)
{
	struct ledtrigger_t * trigger = (struct ledtrigger_t *)dev->priv;
	struct ledtrigger_heartbeat_pdata_t * pdat = (struct ledtrigger_heartbeat_pdata_t *)trigger->priv;

	timer_cancel(&pdat->timer);
}

static void ledtrigger_heartbeat_resume(struct device_t * dev)
{
	struct ledtrigger_t * trigger = (struct ledtrigger_t *)dev->priv;
	struct ledtrigger_heartbeat_pdata_t * pdat = (struct ledtrigger_heartbeat_pdata_t *)trigger->priv;

	timer_start(&pdat->timer, ms_to_ktime(100));
}

static struct driver_t ledtrigger_heartbeat = {
	.name		= "ledtrigger-heartbeat",
	.probe		= ledtrigger_heartbeat_probe,
	.remove		= ledtrigger_heartbeat_remove,
	.suspend	= ledtrigger_heartbeat_suspend,
	.resume		= ledtrigger_heartbeat_resume,
};

static void ledtrigger_heartbeat_driver_init(void)
{
	register_driver(&ledtrigger_heartbeat);
}

static void ledtrigger_heartbeat_driver_exit(void)
{
	unregister_driver(&ledtrigger_heartbeat);
}

driver_initcall(ledtrigger_heartbeat_driver_init);
driver_exitcall(ledtrigger_heartbeat_driver_exit);
