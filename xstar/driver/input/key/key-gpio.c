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

#include <kernel/core/logger.h>
#include <kernel/window/event.h>
#include <driver/interrupt/interrupt.h>
#include <driver/gpio/gpio.h>
#include <driver/input/input.h>

struct gpio_key_t {
	int gpio;
	int gpiocfg;
	int active_low;
	int keycode;
	int state;
	void * priv;
};

struct key_gpio_pdata_t {
	struct gpio_key_t * keys;
	int nkeys;
};

static void key_gpio_interrupt_function(void * data)
{
	struct gpio_key_t * key = (struct gpio_key_t *)(data);
	struct input_t * input = (struct input_t *)(key->priv);
	enum event_type_t type;
	int val;

	val = gpio_get_value(key->gpio);
	if(val != key->state)
	{
		if(key->active_low)
			type = val ? EVENT_TYPE_KEY_UP : EVENT_TYPE_KEY_DOWN;
		else
			type = val ? EVENT_TYPE_KEY_DOWN : EVENT_TYPE_KEY_UP;
		if(type == EVENT_TYPE_KEY_DOWN)
			push_event_key_down(input, key->keycode);
		else if(type == EVENT_TYPE_KEY_UP)
			push_event_key_up(input, key->keycode);
		key->state = val;
	}
}

static int key_gpio_ioctl(struct input_t * input, const char * cmd, void * arg)
{
	return -1;
}

static struct device_t * key_gpio_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct key_gpio_pdata_t * pdat;
	struct gpio_key_t * keys;
	struct input_t * input;
	struct device_t * dev;
	struct dtnode_t o;
	int nkeys, i;

	if((nkeys = dt_read_array_length(n, "keys")) <= 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct key_gpio_pdata_t));
	if(!pdat)
		return NULL;

	keys = xos_mem_malloc(sizeof(struct gpio_key_t) * nkeys);
	if(!keys)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	input = xos_mem_malloc(sizeof(struct input_t));
	if(!input)
	{
		xos_mem_free(pdat);
		xos_mem_free(keys);
		return NULL;
	}

	for(i = 0; i < nkeys; i++)
	{
		dt_read_array_object(n, "keys", i, &o);
		keys[i].gpio = dt_read_int(&o, "gpio", -1);
		keys[i].gpiocfg = dt_read_int(&o, "gpio-config", -1);
		keys[i].active_low = dt_read_bool(&o, "active-low", 0);
		keys[i].keycode = dt_read_int(&o, "key-code", 0);
		keys[i].priv = input;

		gpio_set_direction(keys[i].gpio, GPIO_DIRECTION_INPUT);
		gpio_set_pull(keys[i].gpio, keys[i].active_low ? GPIO_PULL_UP : GPIO_PULL_DOWN);
		if(keys[i].gpiocfg >= 0)
			gpio_set_cfg(keys[i].gpio, keys[i].gpiocfg);
		keys[i].state = gpio_get_value(keys[i].gpio);
	}
	pdat->keys = keys;
	pdat->nkeys = nkeys;

	input->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	input->ioctl = key_gpio_ioctl;
	input->priv = pdat;

	for(i = 0; i < pdat->nkeys; i++)
	{
		if(!request_irq(gpio_to_irq(pdat->keys[i].gpio), key_gpio_interrupt_function, IRQ_TYPE_EDGE_BOTH, &pdat->keys[i]))
			LOG("Fail to request interrupt %d, gpio is %d\r\n", gpio_to_irq(pdat->keys[i].gpio), pdat->keys[i].gpio);
	}

	if(!(dev = register_input(input, drv)))
	{
		for(i = 0; i < pdat->nkeys; i++)
			free_irq(gpio_to_irq(pdat->keys[i].gpio));
		xos_mem_free(pdat->keys);
		free_device_name(input->name);
		xos_mem_free(input->priv);
		xos_mem_free(input);
		return NULL;
	}
	return dev;
}

static void key_gpio_remove(struct device_t * dev)
{
	struct input_t * input = (struct input_t *)dev->priv;
	struct key_gpio_pdata_t * pdat = (struct key_gpio_pdata_t *)input->priv;
	int i;

	if(input)
	{
		unregister_input(input);
		for(i = 0; i < pdat->nkeys; i++)
			free_irq(gpio_to_irq(pdat->keys[i].gpio));
		xos_mem_free(pdat->keys);
		free_device_name(input->name);
		xos_mem_free(input->priv);
		xos_mem_free(input);
	}
}

static void key_gpio_suspend(struct device_t * dev)
{
}

static void key_gpio_resume(struct device_t * dev)
{
}

static struct driver_t key_gpio = {
	.name		= "key-gpio",
	.probe		= key_gpio_probe,
	.remove		= key_gpio_remove,
	.suspend	= key_gpio_suspend,
	.resume		= key_gpio_resume,
};

static void key_gpio_driver_init(void)
{
	register_driver(&key_gpio);
}

static void key_gpio_driver_exit(void)
{
	unregister_driver(&key_gpio);
}

driver_initcall(key_gpio_driver_init);
driver_exitcall(key_gpio_driver_exit);
