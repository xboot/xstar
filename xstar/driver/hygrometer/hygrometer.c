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

#include <driver/hygrometer/hygrometer.h>

static ssize_t hygrometer_read_humidity(struct kobj_t * kobj, void * buf, size_t size)
{
	struct hygrometer_t * hygrometer = (struct hygrometer_t *)kobj->priv;
	int humidity = 0;
	hygrometer_get(hygrometer, &humidity, NULL);
	return xos_sprintf(buf, "%d%%", humidity);
}

static ssize_t hygrometer_read_temperature(struct kobj_t * kobj, void * buf, size_t size)
{
	struct hygrometer_t * hygrometer = (struct hygrometer_t *)kobj->priv;
	int temperature = 0;
	hygrometer_get(hygrometer, NULL, &temperature);
	return xos_sprintf(buf, "%.3f", (float)temperature / 1000.0f);
}

struct hygrometer_t * search_hygrometer(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_HYGROMETER);
	if(!dev)
		return NULL;
	return (struct hygrometer_t *)dev->priv;
}

struct hygrometer_t * search_first_hygrometer(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_HYGROMETER);
	if(!dev)
		return NULL;
	return (struct hygrometer_t *)dev->priv;
}

struct device_t * register_hygrometer(struct hygrometer_t * hygrometer, struct driver_t * drv)
{
	struct device_t * dev;

	if(!hygrometer || !hygrometer->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(hygrometer->name);
	dev->type = DEVICE_TYPE_HYGROMETER;
	dev->driver = drv;
	dev->priv = hygrometer;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "humidity", hygrometer_read_humidity, NULL, hygrometer);
	kobj_add_regular(dev->kobj, "temperature", hygrometer_read_temperature, NULL, hygrometer);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_hygrometer(struct hygrometer_t * hygrometer)
{
	struct device_t * dev;

	if(hygrometer && hygrometer->name)
	{
		dev = search_device(hygrometer->name, DEVICE_TYPE_HYGROMETER);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int hygrometer_get(struct hygrometer_t * hygrometer, int * humidity, int * temperature)
{
	if(hygrometer && hygrometer->get)
		return hygrometer->get(hygrometer, humidity, temperature);
	return 0;
}
