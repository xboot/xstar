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
#include <kernel/time/timer.h>
#include <driver/gpio/gpio.h>
#include <driver/limiter/limiter.h>

struct limiter_gpio_polled_pdata_t {
	struct timer_t timer;
	int interval;
	int gpio;
	int gpiocfg;
	int active_low;
	int status;
	int buffer[4];
	int bufidx;
};

static int limiter_gpio_polled_timer_function(struct timer_t * timer, void * data)
{
	struct limiter_t * limiter = (struct limiter_t *)(data);
	struct limiter_gpio_polled_pdata_t * pdat = (struct limiter_gpio_polled_pdata_t *)limiter->priv;
	int status;

	if(gpio_get_value(pdat->gpio))
		status = pdat->active_low ? 0 : 1;
	else
		status = pdat->active_low ? 1 : 0;
	pdat->buffer[pdat->bufidx] = status;
	pdat->bufidx = (pdat->bufidx + 1) & 0x3;

	if(pdat->status != status)
	{
		if((pdat->buffer[0] == status) && (pdat->buffer[1] == status) && (pdat->buffer[2] == status) && (pdat->buffer[3] == status))
		{
			pdat->status = status;
			if(status)
				psub_publish("limiter.triggered", limiter);
			else
				psub_publish("limiter.released", limiter);
		}
	}
	timer_forward(timer, ms_to_ktime(pdat->interval));
	return 1;
}

static int limiter_gpio_polled_get(struct limiter_t * limiter)
{
	struct limiter_gpio_polled_pdata_t * pdat = (struct limiter_gpio_polled_pdata_t *)limiter->priv;
	return pdat->status;
}

static struct device_t * limiter_gpio_polled_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct limiter_gpio_polled_pdata_t * pdat;
	struct limiter_t * limiter;
	struct device_t * dev;

	if(!gpio_is_valid(dt_read_int(n, "gpio", -1)))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct limiter_gpio_polled_pdata_t));
	if(!pdat)
		return NULL;

	limiter = xos_mem_malloc(sizeof(struct limiter_t));
	if(!limiter)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	timer_init(&pdat->timer, limiter_gpio_polled_timer_function, limiter);
	pdat->interval = dt_read_int(n, "poll-interval-ms", 10);
	pdat->gpio = dt_read_int(n, "gpio", -1);
	pdat->gpiocfg = dt_read_int(n, "gpio-config", -1);
	pdat->active_low = dt_read_bool(n, "active-low", 0);

	limiter->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	limiter->get = limiter_gpio_polled_get;
	limiter->priv = pdat;

	gpio_set_direction(pdat->gpio, GPIO_DIRECTION_INPUT);
	gpio_set_pull(pdat->gpio, pdat->active_low ? GPIO_PULL_UP : GPIO_PULL_DOWN);
	if(pdat->gpiocfg >= 0)
		gpio_set_cfg(pdat->gpio, pdat->gpiocfg);
	if(gpio_get_value(pdat->gpio))
		pdat->status = pdat->active_low ? 0 : 1;
	else
		pdat->status = pdat->active_low ? 1 : 0;
	pdat->buffer[0] = pdat->status;
	pdat->buffer[1] = pdat->status;
	pdat->buffer[2] = pdat->status;
	pdat->buffer[3] = pdat->status;
	pdat->bufidx = 0;
	timer_start(&pdat->timer, ms_to_ktime(pdat->interval));

	if(!(dev = register_limiter(limiter, drv)))
	{
		timer_cancel(&pdat->timer);
		free_device_name(limiter->name);
		xos_mem_free(limiter->priv);
		xos_mem_free(limiter);
		return NULL;
	}
	return dev;
}

static void limiter_gpio_polled_remove(struct device_t * dev)
{
	struct limiter_t * limiter = (struct limiter_t *)dev->priv;
	struct limiter_gpio_polled_pdata_t * pdat = (struct limiter_gpio_polled_pdata_t *)limiter->priv;

	if(limiter)
	{
		unregister_limiter(limiter);
		timer_cancel(&pdat->timer);
		free_device_name(limiter->name);
		xos_mem_free(limiter->priv);
		xos_mem_free(limiter);
	}
}

static void limiter_gpio_polled_suspend(struct device_t * dev)
{
	struct limiter_t * limiter = (struct limiter_t *)dev->priv;
	struct limiter_gpio_polled_pdata_t * pdat = (struct limiter_gpio_polled_pdata_t *)limiter->priv;

	timer_cancel(&pdat->timer);
}

static void limiter_gpio_polled_resume(struct device_t * dev)
{
	struct limiter_t * limiter = (struct limiter_t *)dev->priv;
	struct limiter_gpio_polled_pdata_t * pdat = (struct limiter_gpio_polled_pdata_t *)limiter->priv;

	timer_start(&pdat->timer, ms_to_ktime(pdat->interval));
}

static struct driver_t limiter_gpio_polled = {
	.name		= "limiter-gpio-polled",
	.probe		= limiter_gpio_polled_probe,
	.remove		= limiter_gpio_polled_remove,
	.suspend	= limiter_gpio_polled_suspend,
	.resume		= limiter_gpio_polled_resume,
};

static void limiter_gpio_polled_driver_init(void)
{
	register_driver(&limiter_gpio_polled);
}

static void limiter_gpio_polled_driver_exit(void)
{
	unregister_driver(&limiter_gpio_polled);
}

driver_initcall(limiter_gpio_polled_driver_init);
driver_exitcall(limiter_gpio_polled_driver_exit);
