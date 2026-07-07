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

#include <driver/pressure/pressure.h>

static ssize_t pressure_read_pressure(struct kobj_t * kobj, void * buf, size_t size)
{
	struct pressure_t * p = (struct pressure_t *)kobj->priv;
	int pressure = 0;
	pressure_get(p, &pressure, NULL);
	return xos_sprintf(buf, "%dPa", pressure);
}

static ssize_t pressure_read_temperature(struct kobj_t * kobj, void * buf, size_t size)
{
	struct pressure_t * p = (struct pressure_t *)kobj->priv;
	int temperature = 0;
	pressure_get(p, NULL, &temperature);
	return xos_sprintf(buf, "%.3f", (float)temperature / 1000.0f);
}

struct pressure_t * search_pressure(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_PRESSURE);
	if(!dev)
		return NULL;
	return (struct pressure_t *)dev->priv;
}

struct pressure_t * search_first_pressure(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_PRESSURE);
	if(!dev)
		return NULL;
	return (struct pressure_t *)dev->priv;
}

struct device_t * register_pressure(struct pressure_t * p, struct driver_t * drv)
{
	struct device_t * dev;

	if(!p || !p->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(p->name);
	dev->type = DEVICE_TYPE_PRESSURE;
	dev->driver = drv;
	dev->priv = p;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "pressure", pressure_read_pressure, NULL, p);
	kobj_add_regular(dev->kobj, "temperature", pressure_read_temperature, NULL, p);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_pressure(struct pressure_t * p)
{
	struct device_t * dev;

	if(p && p->name)
	{
		dev = search_device(p->name, DEVICE_TYPE_PRESSURE);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int pressure_get(struct pressure_t * p, int * pressure, int * temperature)
{
	if(p && p->get)
		return p->get(p, pressure, temperature);
	return 0;
}
